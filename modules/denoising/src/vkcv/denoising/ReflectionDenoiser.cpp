
#include "vkcv/denoising/ReflectionDenoiser.hpp"

#include <vkcv/File.hpp>
#include <vkcv/shader/SlangCompiler.hpp>

#include "ffx_denoiser_reflections_common.h.hxx"
#include "ffx_denoiser_reflections_config.h.hxx"
#include "ffx_denoiser_reflections_prefilter.h.hxx"
#include "ffx_denoiser_reflections_reproject.h.hxx"
#include "ffx_denoiser_reflections_resolve_temporal.h.hxx"

#include "Common.hlsl.hxx"
#include "Prefilter.hlsl.hxx"
#include "Reproject.hlsl.hxx"
#include "ResolveTemporal.hlsl.hxx"

namespace vkcv::denoising {
	
	static DescriptorBindings getDescriptorBindings(uint32_t step) {
		DescriptorBindings descriptorBindings = {};
		uint32_t inputs, outputs;

		switch (step) {
			case 0:
				{
					auto binding = DescriptorBinding {
						0,
						DescriptorType::UNIFORM_BUFFER,
						1,
						ShaderStage::COMPUTE,
						false,
						false
					};

					descriptorBindings.insert(std::make_pair(0, binding));
				}

				return descriptorBindings;
			case 1:
				inputs = 7;
				outputs = 3;
				break;
			case 2:
				inputs = 13;
				outputs = 4;
				break;
			case 3:
				inputs = 6;
				outputs = 3;
				break;
			default:
				return descriptorBindings;
		}

		for (uint32_t i = 0; i < inputs; i++) {
			auto input_binding = DescriptorBinding {
				i,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false,
				false
			};

			descriptorBindings.insert(std::make_pair(i, input_binding));
		}

		{
			auto sampler_binding = DescriptorBinding {
				inputs + 0,
				DescriptorType::SAMPLER,
				1,
				ShaderStage::COMPUTE,
				false,
				false
			};

			descriptorBindings.insert(std::make_pair(inputs + 0, sampler_binding));
		}

		for (uint32_t i = 0; i < outputs; i++) {
			auto output_binding = DescriptorBinding {
				inputs + 1 + i,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false,
				false
			};

			descriptorBindings.insert(std::make_pair(inputs + 1 + i, output_binding));
		}

		{
			auto buffer_binding = DescriptorBinding {
				inputs + 1 + outputs,
				DescriptorType::STORAGE_BUFFER,
				1,
				ShaderStage::COMPUTE,
				false,
				false
			};

			descriptorBindings.insert(std::make_pair(inputs + 1 + outputs, buffer_binding));
		}
		
		return descriptorBindings;
	}
	
	ReflectionDenoiser::ReflectionDenoiser(Core &core) :
	Denoiser(core),

	m_prefilterPipeline(),
	m_reprojectPipeline(),
	m_resolveTemporalPipeline(),

	m_commonDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings(0))),
	m_commonDescriptorSet(m_core.createDescriptorSet(m_commonDescriptorSetLayout)),

	m_prefilterDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings(1))),
	m_prefilterDescriptorSet(m_core.createDescriptorSet(m_prefilterDescriptorSetLayout)),

	m_reprojectDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings(2))),
	m_reprojectDescriptorSet(m_core.createDescriptorSet(m_reprojectDescriptorSetLayout)),

	m_resolveTemporalDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings(3))),
	m_resolveTemporalDescriptorSet(m_core.createDescriptorSet(m_resolveTemporalDescriptorSetLayout))
	{
		vkcv::shader::SlangCompiler compiler (vkcv::shader::SlangCompileProfile::HLSL);
		
		{
			ShaderProgram program;
			compiler.compileSourceWithHeaders(
					vkcv::ShaderStage::COMPUTE,
					PREFILTER_HLSL_SHADER,
					{
						{ "ffx_denoiser_reflections_common.h", FFX_DENOISER_REFLECTIONS_COMMON_H_SHADER },
						{ "ffx_denoiser_reflections_config.h", FFX_DENOISER_REFLECTIONS_CONFIG_H_SHADER },
						{ "ffx_denoiser_reflections_prefilter.h", FFX_DENOISER_REFLECTIONS_PREFILTER_H_SHADER },
						{ "Common.hlsl", COMMON_HLSL_SHADER }
					},
					[&program](vkcv::ShaderStage shaderStage,
							   const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);
			
			m_prefilterPipeline = m_core.createComputePipeline({ program, {
				m_commonDescriptorSetLayout,
				m_prefilterDescriptorSetLayout
			}});
			
			DescriptorWrites writes;
			m_core.writeDescriptorSet(m_prefilterDescriptorSet, writes);
		}
		
		{
			ShaderProgram program;
			compiler.compileSourceWithHeaders(
					vkcv::ShaderStage::COMPUTE,
					REPROJECT_HLSL_SHADER,
					{
							{ "ffx_denoiser_reflections_common.h", FFX_DENOISER_REFLECTIONS_COMMON_H_SHADER },
							{ "ffx_denoiser_reflections_config.h", FFX_DENOISER_REFLECTIONS_CONFIG_H_SHADER },
							{ "ffx_denoiser_reflections_reproject.h", FFX_DENOISER_REFLECTIONS_REPROJECT_H_SHADER },
							{ "Common.hlsl", COMMON_HLSL_SHADER }
					},
					[&program](vkcv::ShaderStage shaderStage,
							   const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);
			
			m_reprojectPipeline = m_core.createComputePipeline({ program, {
				m_commonDescriptorSetLayout,
				m_reprojectDescriptorSetLayout
			}});
			
			DescriptorWrites writes;
			m_core.writeDescriptorSet(m_reprojectDescriptorSet, writes);
		}
		
		{
			ShaderProgram program;
			compiler.compileSourceWithHeaders(
					vkcv::ShaderStage::COMPUTE,
					RESOLVETEMPORAL_HLSL_SHADER,
					{
							{ "ffx_denoiser_reflections_common.h", FFX_DENOISER_REFLECTIONS_COMMON_H_SHADER },
							{ "ffx_denoiser_reflections_config.h", FFX_DENOISER_REFLECTIONS_CONFIG_H_SHADER },
							{ "ffx_denoiser_reflections_resolve_temporal.h", FFX_DENOISER_REFLECTIONS_RESOLVE_TEMPORAL_H_SHADER },
							{ "Common.hlsl", COMMON_HLSL_SHADER }
					},
					[&program](vkcv::ShaderStage shaderStage,
							   const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);
			
			m_resolveTemporalPipeline = m_core.createComputePipeline({ program, {
				m_commonDescriptorSetLayout,
				m_resolveTemporalDescriptorSetLayout
			}});
			
			DescriptorWrites writes;
			m_core.writeDescriptorSet(m_resolveTemporalDescriptorSet, writes);
		}
	}
	
	void ReflectionDenoiser::recordDenoising(const CommandStreamHandle &cmdStream, const ImageHandle &input,
											 const ImageHandle &output) {
		
	}
	
}
