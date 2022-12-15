
#include "vkcv/denoising/ReflectionDenoiser.hpp"

#include <vkcv/File.hpp>
#include <vkcv/shader/HLSLCompiler.hpp>

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
	
	static DescriptorBindings getDescriptorBindings() {
		DescriptorBindings descriptorBindings = {};
		
		return descriptorBindings;
	}
	
	ReflectionDenoiser::ReflectionDenoiser(Core &core) :
	Denoiser(core),
	
	m_prefilterPipeline(),
	m_reprojectPipeline(),
	m_resolveTemporalPipeline(),
	
	m_prefilterDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_prefilterDescriptorSet(m_core.createDescriptorSet(m_prefilterDescriptorSetLayout)),
	
	m_reprojectDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_reprojectDescriptorSet(m_core.createDescriptorSet(m_reprojectDescriptorSetLayout)),
	
	m_resolveTemporalDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_resolveTemporalDescriptorSet(m_core.createDescriptorSet(m_resolveTemporalDescriptorSetLayout))
	{
		vkcv::shader::HLSLCompiler compiler;
		
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
