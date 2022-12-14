
#include "vkcv/denoising/ShadowDenoiser.hpp"

#include <vkcv/File.hpp>
#include <vkcv/shader/HLSLCompiler.hpp>

#include "ffx_denoiser_shadows_filter.h.hxx"
#include "ffx_denoiser_shadows_prepare.h.hxx"
#include "ffx_denoiser_shadows_tileclassification.h.hxx"
#include "ffx_denoiser_shadows_util.h.hxx"

#include "shadowDenoiser_filter.hlsl.hxx"
#include "shadowDenoiser_prepare.hlsl.hxx"
#include "shadowDenoiser_tileClassification.hlsl.hxx"

namespace vkcv::denoising {
	
	static DescriptorBindings getDescriptorBindings() {
		DescriptorBindings descriptorBindings = {};
		
		return descriptorBindings;
	}
	
	ShadowDenoiser::ShadowDenoiser(Core &core) :
	Denoiser(core),
	
	m_filterPipeline(),
	m_preparePipeline(),
	m_tileClassificationPipeline(),
	
	m_filterDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_filterDescriptorSet(m_core.createDescriptorSet(m_filterDescriptorSetLayout)),
	
	m_prepareDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_prepareDescriptorSet(m_core.createDescriptorSet(m_prepareDescriptorSetLayout)),
	
	m_tileClassificationDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_tileClassificationDescriptorSet(m_core.createDescriptorSet(m_tileClassificationDescriptorSetLayout))
	{
		vkcv::shader::HLSLCompiler compiler;
		
		{
			ShaderProgram program;
			compiler.compileSourceWithHeaders(
					vkcv::ShaderStage::COMPUTE,
					SHADOWDENOISER_FILTER_HLSL_SHADER,
					{
							{ "ffx_denoiser_shadows_filter.h", FFX_DENOISER_SHADOWS_FILTER_H_SHADER },
							{ "ffx_denoiser_shadows_util.h", FFX_DENOISER_SHADOWS_UTIL_H_SHADER }
					},
					[&program](vkcv::ShaderStage shaderStage,
							   const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);
			
			m_filterPipeline = m_core.createComputePipeline({ program, {
					m_filterDescriptorSetLayout
			}});
			
			DescriptorWrites writes;
			m_core.writeDescriptorSet(m_filterDescriptorSet, writes);
		}
		
		{
			ShaderProgram program;
			compiler.compileSourceWithHeaders(
					vkcv::ShaderStage::COMPUTE,
					SHADOWDENOISER_PREPARE_HLSL_SHADER,
					{
						{ "ffx_denoiser_shadows_prepare.h", FFX_DENOISER_SHADOWS_PREPARE_H_SHADER },
						{ "ffx_denoiser_shadows_util.h", FFX_DENOISER_SHADOWS_UTIL_H_SHADER }
					},
					[&program](vkcv::ShaderStage shaderStage,
							   const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);
			
			m_preparePipeline = m_core.createComputePipeline({ program, {
					m_prepareDescriptorSetLayout
			}});
			
			DescriptorWrites writes;
			m_core.writeDescriptorSet(m_prepareDescriptorSet, writes);
		}
		
		{
			ShaderProgram program;
			compiler.compileSourceWithHeaders(
					vkcv::ShaderStage::COMPUTE,
					SHADOWDENOISER_TILECLASSIFICATION_HLSL_SHADER,
					{
							{ "ffx_denoiser_shadows_tileclassification.h", FFX_DENOISER_SHADOWS_TILECLASSIFICATION_H_SHADER },
							{ "ffx_denoiser_shadows_util.h", FFX_DENOISER_SHADOWS_UTIL_H_SHADER }
					},
					[&program](vkcv::ShaderStage shaderStage,
							   const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);
			
			m_tileClassificationPipeline = m_core.createComputePipeline({ program, {
					m_tileClassificationDescriptorSetLayout
			}});
			
			DescriptorWrites writes;
			m_core.writeDescriptorSet(m_tileClassificationDescriptorSet, writes);
		}
	}
	
	void ShadowDenoiser::recordDenoising(const CommandStreamHandle &cmdStream,
										 const ImageHandle &input,
										 const ImageHandle &output) {
		
	}
	
}
