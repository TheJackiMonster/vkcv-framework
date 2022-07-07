
#include "vkcv/algorithm/SinglePassDownsampler.hpp"

#include <cstdint>
#include <cmath>

#define A_CPU 1
#include <ffx_a.h>
#include <ffx_spd.h>

#include "ffx_a.h.hxx"
#include "ffx_spd.h.hxx"
#include "SPDIntegration.glsl.hxx"
#include "SPDIntegrationLinearSampler.glsl.hxx"

#include <vkcv/ComputePipelineConfig.hpp>
#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

namespace vkcv::algorithm {
	
	#define SPD_MAX_MIP_LEVELS 12
	
	struct SPDConstants {
		int mips;
		int numWorkGroupsPerSlice;
		int workGroupOffset[2];
	};
	
	struct SPDConstantsSampler {
		int mips;
		int numWorkGroupsPerSlice;
		int workGroupOffset[2];
		float invInputSize[2];
	};
	
	static DescriptorBindings getDescriptorBindings(const SamplerHandle &sampler) {
		DescriptorBindings descriptorBindings = {};
		
		auto binding_0 = DescriptorBinding {
				0,
				DescriptorType::IMAGE_STORAGE,
				SPD_MAX_MIP_LEVELS + 1,
				ShaderStage::COMPUTE,
				false,
				true
		};
		
		auto binding_1 = DescriptorBinding {
				1,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false,
				false
		};
		
		auto binding_2 = DescriptorBinding{
				2,
				DescriptorType::STORAGE_BUFFER,
				1,
				ShaderStage::COMPUTE,
				false,
				false
		};
		
		auto binding_3 = DescriptorBinding{
				3,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false,
				false
		};
		
		auto binding_4 = DescriptorBinding{
				4,
				DescriptorType::SAMPLER,
				1,
				ShaderStage::COMPUTE,
				false,
				false
		};
		
		descriptorBindings.insert(std::make_pair(0, binding_0));
		descriptorBindings.insert(std::make_pair(1, binding_1));
		descriptorBindings.insert(std::make_pair(2, binding_2));
		
		if (sampler) {
			descriptorBindings.insert(std::make_pair(3, binding_3));
			descriptorBindings.insert(std::make_pair(4, binding_4));
		}
		
		return descriptorBindings;
	}
	
	static bool writeShaderCode(const std::filesystem::path &shaderPath, const std::string& code) {
		std::ofstream file (shaderPath.string(), std::ios::out);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)", shaderPath.string().c_str());
			return false;
		}
		
		file.seekp(0);
		file.write(code.c_str(), static_cast<std::streamsize>(code.length()));
		file.close();
		
		return true;
	}
	
	static bool compileSPDShader(vkcv::shader::GLSLCompiler& compiler,
								 const std::string &source,
								 const shader::ShaderCompiledFunction& compiled) {
		std::filesystem::path directory = generateTemporaryDirectoryPath();
		
		if (!std::filesystem::create_directory(directory)) {
			vkcv_log(LogLevel::ERROR, "The directory could not be created (%s)", directory.string().c_str());
			return false;
		}
		
		if (!writeShaderCode(directory / "ffx_a.h", FFX_A_H_SHADER)) {
			return false;
		}
		
		if (!writeShaderCode(directory / "ffx_spd.h", FFX_SPD_H_SHADER)) {
			return false;
		}
		
		return compiler.compileSource(
				vkcv::ShaderStage::COMPUTE,
				source.c_str(),
				[&directory, &compiled] (vkcv::ShaderStage shaderStage,
										 const std::filesystem::path& path) {
					if (compiled) {
						compiled(shaderStage, path);
					}
					
					std::filesystem::remove_all(directory);
				}, directory
		);
	}
	
	SinglePassDownsampler::SinglePassDownsampler(Core &core,
												 const SamplerHandle &sampler) :
		 vkcv::Downsampler(core),
		 m_pipeline(),
		
		 m_descriptorSetLayout(),
		 m_descriptorSets(),
		
		 m_globalCounter(buffer<uint32_t>(
				 m_core,
				 vkcv::BufferType::STORAGE,
				 6
		 )),
		 
		 m_sampler(sampler) {
		const auto& featureManager = m_core.getContext().getFeatureManager();
		
		const bool partialBound = featureManager.checkFeatures<vk::PhysicalDeviceDescriptorIndexingFeatures>(
				vk::StructureType::ePhysicalDeviceDescriptorIndexingFeatures,
				[](const vk::PhysicalDeviceDescriptorIndexingFeatures& features) {
					return features.descriptorBindingPartiallyBound;
				}
		);
		
		if (!partialBound) {
			return;
		}
		
		m_descriptorSetLayout = m_core.createDescriptorSetLayout(getDescriptorBindings(sampler));
		
		vkcv::shader::GLSLCompiler compiler (vkcv::shader::GLSLCompileTarget::SUBGROUP_OP);
		
		vk::PhysicalDeviceSubgroupProperties subgroupProperties;
		
		vk::PhysicalDeviceProperties2 properties2;
		properties2.pNext = &subgroupProperties;
		m_core.getContext().getPhysicalDevice().getProperties2(&properties2);
		
		const bool no_subgroup_quad = !(subgroupProperties.supportedOperations & vk::SubgroupFeatureFlagBits::eQuad);
		
		if (no_subgroup_quad) {
			compiler.setDefine("SPD_NO_WAVE_OPERATIONS", "1");
		}
		
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
				) &&
				((no_subgroup_quad) ||
				(featureManager.checkFeatures<vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures>(
						vk::StructureType::ePhysicalDeviceShaderSubgroupExtendedTypesFeatures,
						[](const vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures& features) {
							return features.shaderSubgroupExtendedTypes;
						}
				)))
		);
		
		if (float16Support) {
			compiler.setDefine("A_HALF", "1");
			compiler.setDefine("SPD_PACKED_ONLY", "1");
		}
		
		ShaderProgram program;
		if (m_sampler) {
			compileSPDShader(
					compiler,
					SPDINTEGRATIONLINEARSAMPLER_GLSL_SHADER,
					[&program](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);
		} else {
			compileSPDShader(
					compiler,
					SPDINTEGRATION_GLSL_SHADER,
					[&program](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
						program.addShader(shaderStage, path);
					}
			);
		}
		
		m_pipeline = m_core.createComputePipeline(ComputePipelineConfig(
			program,
			{ m_descriptorSetLayout }
		));
		
		uint32_t zeroes [m_globalCounter.getCount()];
		memset(zeroes, 0, m_globalCounter.getSize());
		m_globalCounter.fill(zeroes);
	}
	
	void SinglePassDownsampler::recordDownsampling(const CommandStreamHandle &cmdStream,
												   const ImageHandle &image) {
		Downsampler& fallback = m_core.getDownsampler();
		
		if (m_pipeline) {
			fallback.recordDownsampling(cmdStream, image);
			return;
		}
		
		const uint32_t mipLevels = m_core.getImageMipLevels(image);
		const uint32_t depth = m_core.getImageDepth(image);
		
		m_core.prepareImageForSampling(cmdStream, image);
		
		if ((mipLevels < 4) || (depth > 1) || (!m_core.isImageSupportingStorage(image))) {
			fallback.recordDownsampling(cmdStream, image);
			return;
		}
		
		auto descriptorSet = m_core.createDescriptorSet(m_descriptorSetLayout);
		
		vkcv::DescriptorWrites writes;
		writes.writeStorageImage(1, image, 6, 1, true);
		writes.writeStorageBuffer(2, m_globalCounter.getHandle());
		
		if (m_sampler) {
			writes.writeStorageImage(0, image, 1, mipLevels - 1, true);
			
			writes.writeSampledImage(3, image, 0, false, 0, 1, true);
			writes.writeSampler(4, m_sampler);
		} else {
			writes.writeStorageImage(0, image, 0, mipLevels, true);
		}
		
		m_core.writeDescriptorSet(descriptorSet, writes);
		m_descriptorSets.push_back(descriptorSet);
		
		m_core.recordCommandsToStream(cmdStream, nullptr, [this]() {
			m_descriptorSets.erase(m_descriptorSets.begin());
		});
		
		varAU2(dispatchThreadGroupCountXY);
		varAU2(workGroupOffset);
		varAU2(numWorkGroupsAndMips);
		varAU4(rectInfo) = initAU4(
				0,
				0,
				m_core.getImageWidth(image),
				m_core.getImageHeight(image)
		);
		
		SpdSetup(
				dispatchThreadGroupCountXY,
				workGroupOffset,
				numWorkGroupsAndMips,
				rectInfo
		);
		
		if (m_sampler) {
			m_core.prepareImageForSampling(cmdStream, image, 1);
			m_core.prepareImageForStorage(cmdStream, image, m_core.getImageMipLevels(image) - 1, 1);
		} else {
			m_core.prepareImageForStorage(cmdStream, image);
		}
		
		uint32_t dispatch [3];
		dispatch[0] = dispatchThreadGroupCountXY[0];
		dispatch[1] = dispatchThreadGroupCountXY[1];
		dispatch[2] = m_core.getImageArrayLayers(image);
		
		vkcv::PushConstants pushConstants = (m_sampler?
				vkcv::pushConstants<SPDConstantsSampler>() :
				vkcv::pushConstants<SPDConstants>()
		);
		
		if (m_sampler) {
			SPDConstantsSampler data;
			data.numWorkGroupsPerSlice = numWorkGroupsAndMips[0];
			data.mips = numWorkGroupsAndMips[1];
			data.workGroupOffset[0] = workGroupOffset[0];
			data.workGroupOffset[1] = workGroupOffset[1];
			data.invInputSize[0] = 1.0f / m_core.getImageWidth(image);
			data.invInputSize[1] = 1.0f / m_core.getImageHeight(image);
			
			pushConstants.appendDrawcall<SPDConstantsSampler>(data);
		} else {
			SPDConstants data;
			data.numWorkGroupsPerSlice = numWorkGroupsAndMips[0];
			data.mips = numWorkGroupsAndMips[1];
			data.workGroupOffset[0] = workGroupOffset[0];
			data.workGroupOffset[1] = workGroupOffset[1];
			
			pushConstants.appendDrawcall<SPDConstants>(data);
		}
		
		m_core.recordComputeDispatchToCmdStream(cmdStream, m_pipeline, dispatch, {
			vkcv::DescriptorSetUsage(0, descriptorSet)
		}, pushConstants);
		
		if (m_sampler) {
			m_core.prepareImageForSampling(cmdStream, image, mipLevels - 1, 1);
		} else {
			m_core.prepareImageForSampling(cmdStream, image);
		}
	}
	
}
