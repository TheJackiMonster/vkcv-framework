
#include "vkcv/upscaling/NISUpscaling.hpp"

#include <NIS_Config.h>

#include "NIS_Main.glsl.hxx"
#include "NIS_Scaler.h.hxx"

#include <vkcv/File.hpp>
#include <vkcv/Image.hpp>
#include <vkcv/Logger.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

namespace vkcv::upscaling {
	
	static DescriptorBindings getDescriptorBindings() {
		DescriptorBindings descriptorBindings = {};
		
		auto binding_0 = DescriptorBinding {
				0,
				DescriptorType::UNIFORM_BUFFER_DYNAMIC,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_1 = DescriptorBinding{
				1,
				DescriptorType::SAMPLER,
				1,
				ShaderStage::COMPUTE
		};
		
		auto binding_2 = DescriptorBinding {
				2,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_3 = DescriptorBinding{
				3,
				DescriptorType::IMAGE_STORAGE,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_4 = DescriptorBinding {
				4,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		auto binding_5 = DescriptorBinding {
				5,
				DescriptorType::IMAGE_SAMPLED,
				1,
				ShaderStage::COMPUTE,
				false
		};
		
		descriptorBindings.insert(std::make_pair(0, binding_0));
		descriptorBindings.insert(std::make_pair(1, binding_1));
		descriptorBindings.insert(std::make_pair(2, binding_2));
		descriptorBindings.insert(std::make_pair(3, binding_3));
		descriptorBindings.insert(std::make_pair(4, binding_4));
		descriptorBindings.insert(std::make_pair(5, binding_5));
		
		return descriptorBindings;
	}
	
	static ImageHandle createFilterImage(Core &core,
										 const void* data) {
		const size_t rowPitch = kFilterSize * 2;
		const size_t imageSize = rowPitch * kPhaseCount;
		
		Image image = vkcv::image(
				core,
				vk::Format::eR16G16B16A16Sfloat,
				kFilterSize / 4,
				kPhaseCount
		);
		
		image.fill(data, imageSize);
		return image.getHandle();
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
	
	static bool compileNISShader(vkcv::shader::GLSLCompiler& compiler,
								 const shader::ShaderCompiledFunction& compiled) {
		std::filesystem::path directory = generateTemporaryDirectoryPath();
		
		if (!std::filesystem::create_directory(directory)) {
			vkcv_log(LogLevel::ERROR, "The directory could not be created (%s)", directory.string().c_str());
			return false;
		}
		
		if (!writeShaderCode(directory / "NIS_Scaler.h", NIS_SCALER_H_SHADER)) {
			return false;
		}
		
		return compiler.compileSource(
				vkcv::ShaderStage::COMPUTE,
				NIS_MAIN_GLSL_SHADER.c_str(),
				[&directory, &compiled] (vkcv::ShaderStage shaderStage,
										 const std::filesystem::path& path) {
				if (compiled) {
					compiled(shaderStage, path);
				}
				
				std::filesystem::remove_all(directory);
			}, directory
		);
	}
	
	NISUpscaling::NISUpscaling(Core &core) :
	Upscaling(core),
	m_scalerPipeline(),
	
	m_scalerDescriptorSetLayout(m_core.createDescriptorSetLayout(getDescriptorBindings())),
	m_scalerDescriptorSet(m_core.createDescriptorSet(m_scalerDescriptorSetLayout)),
	
	m_scalerConstants(buffer<uint8_t>(
			m_core,
			BufferType::UNIFORM,
			sizeof(NISConfig),
			BufferMemoryType::HOST_VISIBLE
	)),
	m_sampler(m_core.createSampler(
			SamplerFilterType::LINEAR,
			SamplerFilterType::LINEAR,
			SamplerMipmapMode::NEAREST,
			SamplerAddressMode::CLAMP_TO_EDGE
	)),
	
	m_coefScaleImage(createFilterImage(m_core, coef_scale_fp16)),
	m_coefUsmImage(createFilterImage(m_core, coef_usm_fp16)),
	
	m_blockWidth(0),
	m_blockHeight(0),
	
	m_hdr(false),
	m_sharpness(0.875f) {
		vkcv::shader::GLSLCompiler scalerCompiler;
		
		scalerCompiler.setDefine("NIS_SCALER", "1");
		scalerCompiler.setDefine("NIS_GLSL", "1");
		
		NISOptimizer optimizer (true, NISGPUArchitecture::NVIDIA_Generic);
		
		m_blockWidth = optimizer.GetOptimalBlockWidth();
		m_blockHeight = optimizer.GetOptimalBlockHeight();
		
		scalerCompiler.setDefine("NIS_BLOCK_WIDTH", std::to_string(m_blockWidth));
		scalerCompiler.setDefine("NIS_BLOCK_HEIGHT", std::to_string(m_blockHeight));
		
		const uint32_t threadGroupSize = optimizer.GetOptimalThreadGroupSize();
		
		scalerCompiler.setDefine("NIS_THREAD_GROUP_SIZE", std::to_string(threadGroupSize));
		
		{
			ShaderProgram program;
			compileNISShader(scalerCompiler, [&program](vkcv::ShaderStage shaderStage,
														const std::filesystem::path& path) {
				program.addShader(shaderStage, path);
			});
			
			m_scalerPipeline = m_core.createComputePipeline({program,{
					m_scalerDescriptorSetLayout
			}});
			
			
			DescriptorWrites writes;
			writes.writeUniformBuffer(
					0, m_scalerConstants.getHandle(), true
			);
			
			writes.writeSampler(1, m_sampler);
			writes.writeSampledImage(4, m_coefScaleImage);
			writes.writeSampledImage(5, m_coefUsmImage);
			
			m_core.writeDescriptorSet(m_scalerDescriptorSet, writes);
		}
	}
	
	void NISUpscaling::recordUpscaling(const CommandStreamHandle &cmdStream,
									   const ImageHandle &input,
									   const ImageHandle &output) {
		m_core.recordBeginDebugLabel(cmdStream, "vkcv::upscaling::NISUpscaling", {
				0.0f, 1.0f, 0.0f, 1.0f
		});
		
		const uint32_t inputWidth = m_core.getImageWidth(input);
		const uint32_t inputHeight = m_core.getImageHeight(input);
		
		const uint32_t outputWidth = m_core.getImageWidth(output);
		const uint32_t outputHeight = m_core.getImageHeight(output);
		
		NISConfig config {};
		NVScalerUpdateConfig(
				config,
				m_sharpness,
				0, 0,
				inputWidth,
				inputHeight,
				inputWidth,
				inputHeight,
				0, 0,
				outputWidth,
				outputHeight,
				outputWidth,
				outputHeight,
				m_hdr? NISHDRMode::PQ : NISHDRMode::None
		);
		
		m_scalerConstants.fill(
				reinterpret_cast<uint8_t*>(&config),
				sizeof(config)
		);
		
		DispatchSize dispatch = dispatchInvocations(
				DispatchSize(outputWidth, outputHeight),
				DispatchSize(m_blockWidth, m_blockHeight)
		);
		
		m_core.recordBufferMemoryBarrier(cmdStream, m_scalerConstants.getHandle());
		
		{
			DescriptorWrites writes;
			writes.writeSampledImage(2, input);
			writes.writeStorageImage(3, output);
			
			m_core.writeDescriptorSet(m_scalerDescriptorSet, writes);
		}
		
		m_core.recordComputeDispatchToCmdStream(
				cmdStream,
				m_scalerPipeline,
				dispatch,
				{DescriptorSetUsage(0, m_scalerDescriptorSet, { 0 })},
				PushConstants(0)
		);
		
		m_core.recordEndDebugLabel(cmdStream);
	}
	
	bool NISUpscaling::isHdrEnabled() const {
		return m_hdr;
	}
	
	void NISUpscaling::setHdrEnabled(bool enabled) {
		m_hdr = enabled;
	}
	
	float NISUpscaling::getSharpness() const {
		return m_sharpness;
	}
	
	void NISUpscaling::setSharpness(float sharpness) {
		m_sharpness = (sharpness < 0.0f ? 0.0f : (sharpness > 1.0f ? 1.0f : sharpness));
	}

}
