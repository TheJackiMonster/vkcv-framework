
#include "vkcv/upscaling/FSRUpscaling.hpp"

#include <stdint.h>
#include <math.h>

#define A_CPU 1
#include <ffx_a.h>
#include <ffx_fsr1.h>

#include "ffx_a.h.hxx"
#include "ffx_fsr1.h.hxx"
#include "FSR_Pass.glsl.hxx"

#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>
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
	
	static std::vector<DescriptorBinding> getDescriptorBindings() {
		return std::vector<DescriptorBinding>({
			DescriptorBinding(
					0, DescriptorType::UNIFORM_BUFFER_DYNAMIC,
					1, ShaderStage::COMPUTE
			),
			DescriptorBinding(
					1, DescriptorType::IMAGE_SAMPLED,
					1, ShaderStage::COMPUTE
			),
			DescriptorBinding(
					2, DescriptorType::IMAGE_STORAGE,
					1, ShaderStage::COMPUTE
			),
			DescriptorBinding(
					3, DescriptorType::SAMPLER,
					1, ShaderStage::COMPUTE
			)
		});
	}
	
	template<typename T>
	bool checkFeatures(const vk::BaseInStructure* base, vk::StructureType type, bool (*check)(const T& features)) {
		if (base->sType == type) {
			return check(*reinterpret_cast<const T*>(base));
		} else
		if (base->pNext) {
			return checkFeatures<T>(base->pNext, type, check);
		} else {
			return false;
		}
	}
	
	static bool checkFloat16(const vk::PhysicalDeviceFloat16Int8FeaturesKHR& features) {
		return features.shaderFloat16;
	}
	
	static bool check16Storage(const vk::PhysicalDevice16BitStorageFeaturesKHR& features) {
		return features.storageBuffer16BitAccess;
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
	
	static bool compileFSRShader(vkcv::shader::GLSLCompiler& compiler,
								 const shader::ShaderCompiledFunction& compiled) {
		std::filesystem::path directory = generateTemporaryDirectoryPath();
		
		if (!std::filesystem::create_directory(directory)) {
			vkcv_log(LogLevel::ERROR, "The directory could not be created (%s)", directory.string().c_str());
			return false;
		}
		
		if (!writeShaderCode(directory / "ffx_a.h", FFX_A_H_SHADER)) {
			return false;
		}
		
		if (!writeShaderCode(directory / "ffx_fsr1.h", FFX_FSR1_H_SHADER)) {
			return false;
		}
		
		return compiler.compileSource(vkcv::ShaderStage::COMPUTE,
									  FSR_PASS_GLSL_SHADER.c_str(),
									  [&directory, &compiled] (vkcv::ShaderStage shaderStage,
									  		const std::filesystem::path& path) {
				if (compiled) {
					compiled(shaderStage, path);
				}
				
				std::filesystem::remove_all(directory);
			}, directory
		);
	}
	
	FSRUpscaling::FSRUpscaling(Core& core) :
	Upscaling(core),
	m_easuPipeline(),
	m_rcasPipeline(),
	m_easuDescriptorSet(m_core.createDescriptorSet(getDescriptorBindings())),
	m_rcasDescriptorSet(m_core.createDescriptorSet(getDescriptorBindings())),
	m_easuConstants(m_core.createBuffer<FSRConstants>(
			BufferType::UNIFORM,1,
			BufferMemoryType::HOST_VISIBLE
	)),
	m_rcasConstants(m_core.createBuffer<FSRConstants>(
			BufferType::UNIFORM,1,
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
		
		const auto& features = m_core.getContext().getFeatureManager().getFeatures();
		const bool float16Support = (
				checkFeatures<vk::PhysicalDeviceFloat16Int8FeaturesKHR>(
						reinterpret_cast<const vk::BaseInStructure*>(&features),
						vk::StructureType::ePhysicalDeviceShaderFloat16Int8FeaturesKHR,
						checkFloat16
				) &&
				checkFeatures<vk::PhysicalDevice16BitStorageFeaturesKHR>(
						reinterpret_cast<const vk::BaseInStructure*>(&features),
						vk::StructureType::ePhysicalDevice16BitStorageFeaturesKHR,
						check16Storage
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
			compileFSRShader(easuCompiler, [&program](vkcv::ShaderStage shaderStage,
					const std::filesystem::path& path) {
				program.addShader(shaderStage, path);
			});
			
			m_easuPipeline = m_core.createComputePipeline(program, {
				m_core.getDescriptorSet(m_easuDescriptorSet).layout
			});
			
			DescriptorWrites writes;
			writes.uniformBufferWrites.emplace_back(
					0, m_easuConstants.getHandle(),true
			);
			
			writes.samplerWrites.emplace_back(3, m_sampler);
			
			m_core.writeDescriptorSet(m_easuDescriptorSet, writes);
		}
		
		{
			ShaderProgram program;
			compileFSRShader(rcasCompiler, [&program](vkcv::ShaderStage shaderStage,
					const std::filesystem::path& path) {
				program.addShader(shaderStage, path);
			});
			
			m_rcasPipeline = m_core.createComputePipeline(program, {
				m_core.getDescriptorSet(m_rcasDescriptorSet).layout
			});
			
			DescriptorWrites writes;
			writes.uniformBufferWrites.emplace_back(
					0, m_rcasConstants.getHandle(),true
			);
			
			writes.samplerWrites.emplace_back(3, m_sampler);
			
			m_core.writeDescriptorSet(m_rcasDescriptorSet, writes);
		}
	}
	
	void FSRUpscaling::recordUpscaling(const CommandStreamHandle& cmdStream,
									   const ImageHandle& input,
									   const ImageHandle& output) {
		const uint32_t inputWidth = m_core.getImageWidth(input);
		const uint32_t inputHeight = m_core.getImageHeight(input);
		
		const uint32_t outputWidth = m_core.getImageWidth(output);
		const uint32_t outputHeight = m_core.getImageHeight(output);
		
		if ((!m_intermediateImage) ||
			(outputWidth != m_core.getImageWidth(m_intermediateImage)) ||
			(outputHeight != m_core.getImageHeight(m_intermediateImage))) {
			m_intermediateImage = m_core.createImage(
					m_core.getImageFormat(output),
					outputWidth, outputHeight,1,
					false,
					true
			).getHandle();
			
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
		
		uint32_t dispatch[3];
		dispatch[0] = (outputWidth + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;
		dispatch[1] = (outputHeight + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;
		dispatch[2] = 1;
		
		m_core.recordBufferMemoryBarrier(cmdStream, m_easuConstants.getHandle());
		
		if (rcasEnabled) {
			{
				DescriptorWrites writes;
				writes.sampledImageWrites.emplace_back(1, input);
				writes.storageImageWrites.emplace_back(2, m_intermediateImage);
				
				m_core.writeDescriptorSet(m_easuDescriptorSet, writes);
			}
			{
				DescriptorWrites writes;
				writes.sampledImageWrites.emplace_back(1, m_intermediateImage);
				writes.storageImageWrites.emplace_back(2, output);
				
				m_core.writeDescriptorSet(m_rcasDescriptorSet, writes);
			}
			
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_easuPipeline,
					dispatch,
					{DescriptorSetUsage(0, m_core.getDescriptorSet(
							m_easuDescriptorSet
					).vulkanHandle, { 0 })},
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
					{DescriptorSetUsage(0, m_core.getDescriptorSet(
							m_rcasDescriptorSet
					).vulkanHandle, { 0 })},
					PushConstants(0)
			);
			
			m_core.prepareImageForStorage(cmdStream, m_intermediateImage);
		} else {
			{
				DescriptorWrites writes;
				writes.sampledImageWrites.emplace_back(1, input);
				writes.storageImageWrites.emplace_back(2, output);
				
				m_core.writeDescriptorSet(m_easuDescriptorSet, writes);
			}
			
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_easuPipeline,
					dispatch,
					{DescriptorSetUsage(0, m_core.getDescriptorSet(
							m_easuDescriptorSet
					).vulkanHandle, { 0 })},
					PushConstants(0)
			);
		}
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
