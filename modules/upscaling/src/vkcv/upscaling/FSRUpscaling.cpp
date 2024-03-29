
#include "vkcv/upscaling/FSRUpscaling.hpp"

#include <cstdint>
#include <cmath>

#define A_CPU 1
#include <ffx_a.h>
#include <ffx_fsr1.h>

#include "ffx_a.h.hxx"
#include "ffx_fsr1.h.hxx"
#include "FSR_Pass.glsl.hxx"

#include <vkcv/File.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

namespace vkcv::upscaling {
	
	void getFSRResolution(FSRQualityMode mode,
						  uint32_t outputWidth, uint32_t outputHeight,
						  uint32_t &inputWidth, uint32_t &inputHeight) {
		float scale;
		
		switch (mode) {
			case FSRQualityMode::ULTRA_QUALITY:
				scale = 1.3f;
				break;
			case FSRQualityMode::QUALITY:
				scale = 1.5f;
				break;
			case FSRQualityMode::BALANCED:
				scale = 1.7f;
				break;
			case FSRQualityMode::PERFORMANCE:
				scale = 2.0f;
				break;
			default:
				scale = 1.0f;
				break;
		}
		
		inputWidth = static_cast<uint32_t>(
				std::round(static_cast<float>(outputWidth) / scale)
		);
		
		inputHeight = static_cast<uint32_t>(
				std::round(static_cast<float>(outputHeight) / scale)
		);
	}
	
	float getFSRLodBias(FSRQualityMode mode) {
		switch (mode) {
			case FSRQualityMode::ULTRA_QUALITY:
				return -0.38f;
			case FSRQualityMode::QUALITY:
				return -0.58f;
			case FSRQualityMode::BALANCED:
				return -0.79f;
			case FSRQualityMode::PERFORMANCE:
				return -1.0f;
			default:
				return 0.0f;
		}
	}
	
	static DescriptorBindings getDescriptorBindings() {
		DescriptorBindings descriptorBindings = {};

	    auto binding_0 = DescriptorBinding {
	            0,
	            DescriptorType::UNIFORM_BUFFER_DYNAMIC,
	            1,
	            ShaderStage::COMPUTE,
				false
		};

	    auto binding_1 = DescriptorBinding {
				1,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false
		};

	    auto binding_2 = DescriptorBinding{
				2,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false
		};

	    auto binding_3 = DescriptorBinding{
				3,
				DescriptorType::SAMPLER,
				1,
				ShaderStage::COMPUTE
		};

	    descriptorBindings.insert(std::make_pair(0, binding_0));
	    descriptorBindings.insert(std::make_pair(1, binding_1));
	    descriptorBindings.insert(std::make_pair(2, binding_2));
	    descriptorBindings.insert(std::make_pair(3, binding_3));

	    return descriptorBindings;
	}
	
	FSRUpscaling::FSRUpscaling(Core& core) :
	Upscaling(core),
	m_easuPipeline(),
	m_rcasPipeline(),

	m_easuDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_easuDescriptorSet(m_core.createDescriptorSet(m_easuDescriptorSetLayout)),

	m_rcasDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_rcasDescriptorSet(m_core.createDescriptorSet(m_rcasDescriptorSetLayout)),

	m_easuConstants(buffer<FSRConstants>(
			m_core,
			BufferType::UNIFORM,
			1,
			BufferMemoryType::HOST_VISIBLE
	)),
	m_rcasConstants(buffer<FSRConstants>(
			m_core,
			BufferType::UNIFORM,
			1,
			BufferMemoryType::HOST_VISIBLE
	)),
	m_intermediateImage(),
	m_sampler(m_core.createSampler(
			SamplerFilterType::LINEAR,
			SamplerFilterType::LINEAR,
			SamplerMipmapMode::NEAREST,
			SamplerAddressMode::CLAMP_TO_EDGE
	)),
	
	m_hdr(false),
	m_sharpness(0.875f) {
		vkcv::shader::GLSLCompiler easuCompiler;
		vkcv::shader::GLSLCompiler rcasCompiler;
		
		const auto& featureManager = m_core.getContext().getFeatureManager();
		
		const bool float16Support = (
				featureManager.checkFeatures<vk::PhysicalDeviceFloat16Int8FeaturesKHR>(
						vk::StructureType::ePhysicalDeviceShaderFloat16Int8FeaturesKHR,
						[](const vk::PhysicalDeviceFloat16Int8FeaturesKHR& features) {
							return features.shaderFloat16;
						}
				) &&
				featureManager.checkFeatures<vk::PhysicalDevice16BitStorageFeaturesKHR>(
						vk::StructureType::ePhysicalDevice16BitStorageFeaturesKHR,
						[](const vk::PhysicalDevice16BitStorageFeaturesKHR& features) {
							return features.storageBuffer16BitAccess;
						}
				)
		);
		
		if (!float16Support) {
			easuCompiler.setDefine("SAMPLE_SLOW_FALLBACK", "1");
			rcasCompiler.setDefine("SAMPLE_SLOW_FALLBACK", "1");
		}
		
		easuCompiler.setDefine("SAMPLE_EASU", "1");
		rcasCompiler.setDefine("SAMPLE_RCAS", "1");
		
		{
			ShaderProgram program;
			easuCompiler.compileSourceWithHeaders(
					ShaderStage::COMPUTE,
					FSR_PASS_GLSL_SHADER,
					{
						{ "ffx_a.h", FFX_A_H_SHADER },
						{ "ffx_fsr1.h", FFX_FSR1_H_SHADER }
					},
					[&program](vkcv::ShaderStage shaderStage,
							   const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);

			m_easuPipeline = m_core.createComputePipeline({ program, {
				m_easuDescriptorSetLayout
			}});

			
			DescriptorWrites writes;
			writes.writeUniformBuffer(
					0, m_easuConstants.getHandle(),true
			);
			
			writes.writeSampler(3, m_sampler);
			
			m_core.writeDescriptorSet(m_easuDescriptorSet, writes);
		}
		
		{
			ShaderProgram program;
			rcasCompiler.compileSourceWithHeaders(
					ShaderStage::COMPUTE,
					FSR_PASS_GLSL_SHADER,
					{
							{ "ffx_a.h", FFX_A_H_SHADER },
							{ "ffx_fsr1.h", FFX_FSR1_H_SHADER }
					},
					[&program](vkcv::ShaderStage shaderStage,
							   const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);

			m_rcasPipeline = m_core.createComputePipeline({ program, {
				m_rcasDescriptorSetLayout
			}});

			DescriptorWrites writes;
			writes.writeUniformBuffer(
					0, m_rcasConstants.getHandle(),true
			);
			
			writes.writeSampler(3, m_sampler);
			
			m_core.writeDescriptorSet(m_rcasDescriptorSet, writes);
		}
	}
	
	void FSRUpscaling::recordUpscaling(const CommandStreamHandle& cmdStream,
									   const ImageHandle& input,
									   const ImageHandle& output) {
		m_core.recordBeginDebugLabel(cmdStream, "vkcv::upscaling::FSRUpscaling", {
			1.0f, 0.0f, 0.0f, 1.0f
		});
		
		const uint32_t inputWidth = m_core.getImageWidth(input);
		const uint32_t inputHeight = m_core.getImageHeight(input);
		
		const uint32_t outputWidth = m_core.getImageWidth(output);
		const uint32_t outputHeight = m_core.getImageHeight(output);
		
		if ((!m_intermediateImage) ||
			(outputWidth != m_core.getImageWidth(m_intermediateImage)) ||
			(outputHeight != m_core.getImageHeight(m_intermediateImage))) {
			ImageConfig imageConfig (outputWidth, outputHeight);
			imageConfig.setSupportingStorage(true);
			
			m_intermediateImage = m_core.createImage(
					m_core.getImageFormat(output),
					imageConfig
			);
			
			m_core.prepareImageForStorage(cmdStream, m_intermediateImage);
		}
		
		const bool rcasEnabled = (
				(m_sharpness > +0.0f) &&
				((inputWidth < outputWidth) || (inputHeight < outputHeight))
		);
		
		{
			FSRConstants consts = {};
			
			FsrEasuCon(
					consts.Const0, consts.Const1, consts.Const2, consts.Const3,
					static_cast<AF1>(inputWidth), static_cast<AF1>(inputHeight),
					static_cast<AF1>(inputWidth), static_cast<AF1>(inputHeight),
					static_cast<AF1>(outputWidth), static_cast<AF1>(outputHeight)
			);
			
			consts.Sample[0] = (((m_hdr) && (!rcasEnabled)) ? 1 : 0);
			
			m_easuConstants.fill(&consts);
		}
		
		static const uint32_t threadGroupWorkRegionDim = 16;
		
		DispatchSize dispatch = dispatchInvocations(
				DispatchSize(outputWidth, outputHeight),
				DispatchSize(threadGroupWorkRegionDim, threadGroupWorkRegionDim)
		);
		
		m_core.recordBufferMemoryBarrier(cmdStream, m_easuConstants.getHandle());
		
		if (rcasEnabled) {
			{
				DescriptorWrites writes;
				writes.writeSampledImage(1, input);
				writes.writeStorageImage(2, m_intermediateImage);
				
				m_core.writeDescriptorSet(m_easuDescriptorSet, writes);
			}
			{
				DescriptorWrites writes;
				writes.writeSampledImage(1, m_intermediateImage);
				writes.writeStorageImage(2, output);
				
				m_core.writeDescriptorSet(m_rcasDescriptorSet, writes);
			}
			
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_easuPipeline,
					dispatch,
					{ useDescriptorSet(0, m_easuDescriptorSet, { 0 }) },
					PushConstants(0)
			);
			
			{
				FSRConstants consts = {};
				
				FsrRcasCon(consts.Const0, (1.0f - m_sharpness) * 2.0f);
				consts.Sample[0] = (m_hdr ? 1 : 0);
				
				m_rcasConstants.fill(&consts);
			}
			
			m_core.recordBufferMemoryBarrier(cmdStream, m_rcasConstants.getHandle());
			m_core.prepareImageForSampling(cmdStream, m_intermediateImage);
			
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_rcasPipeline,
					dispatch,
					{ useDescriptorSet(0,m_rcasDescriptorSet, { 0 }) },
					PushConstants(0)
			);
			
			m_core.prepareImageForStorage(cmdStream, m_intermediateImage);
		} else {
			{
				DescriptorWrites writes;
				writes.writeSampledImage(1, input);
				writes.writeStorageImage(2, output);
				
				m_core.writeDescriptorSet(m_easuDescriptorSet, writes);
			}
			
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_easuPipeline,
					dispatch,
					{ useDescriptorSet(0, m_easuDescriptorSet, { 0 }) },
					PushConstants(0)
			);
		}
		
		m_core.recordEndDebugLabel(cmdStream);
	}
	
	bool FSRUpscaling::isHdrEnabled() const {
		return m_hdr;
	}
	
	void FSRUpscaling::setHdrEnabled(bool enabled) {
		m_hdr = enabled;
	}
	
	float FSRUpscaling::getSharpness() const {
		return m_sharpness;
	}
	
	void FSRUpscaling::setSharpness(float sharpness) {
		m_sharpness = (sharpness < 0.0f ? 0.0f : (sharpness > 1.0f ? 1.0f : sharpness));
	}
	
}
