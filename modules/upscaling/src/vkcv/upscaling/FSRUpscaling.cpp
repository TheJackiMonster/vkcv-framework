
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
	m_core(core),
	m_easuPipeline(),
	m_rcasPipeline(),
	m_easuDescriptorSet(),
	m_rcasDescriptorSet(),
	m_constants(m_core.createBuffer<FSRConstants>(
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
	m_sharpness(0.0f) {
		vkcv::shader::GLSLCompiler easuCompiler, rcasCompiler;
		
		easuCompiler.setDefine("SAMPLE_SLOW_FALLBACK", "1");
		easuCompiler.setDefine("SAMPLE_EASU", "1");
		
		rcasCompiler.setDefine("SAMPLE_SLOW_FALLBACK", "1");
		rcasCompiler.setDefine("SAMPLE_RCAS", "1");
		
		{
			ShaderProgram program;
			compileFSRShader(easuCompiler, [&program](vkcv::ShaderStage shaderStage,
					const std::filesystem::path& path) {
				program.addShader(shaderStage, path);
			});
			
			m_easuDescriptorSet = m_core.createDescriptorSet(program.getReflectedDescriptors()[0]);
			m_easuPipeline = m_core.createComputePipeline(program, {
				m_core.getDescriptorSet(m_easuDescriptorSet).layout
			});
			
			DescriptorWrites writes;
			writes.uniformBufferWrites.emplace_back(0, m_constants.getHandle());
			writes.samplerWrites.emplace_back(3, m_sampler);
			
			m_core.writeDescriptorSet(m_easuDescriptorSet, writes);
		}
		
		{
			ShaderProgram program;
			compileFSRShader(rcasCompiler, [&program](vkcv::ShaderStage shaderStage,
					const std::filesystem::path& path) {
				program.addShader(shaderStage, path);
			});
			
			m_rcasDescriptorSet = m_core.createDescriptorSet(program.getReflectedDescriptors()[0]);
			m_rcasPipeline = m_core.createComputePipeline(program, {
				m_core.getDescriptorSet(m_rcasDescriptorSet).layout
			});
			
			DescriptorWrites writes;
			writes.uniformBufferWrites.emplace_back(0, m_constants.getHandle());
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
		
		{
			FSRConstants consts = {};
			
			FsrEasuCon(
					consts.Const0, consts.Const1, consts.Const2, consts.Const3,
					static_cast<AF1>(inputWidth), static_cast<AF1>(inputHeight),
					static_cast<AF1>(inputWidth), static_cast<AF1>(inputHeight),
					static_cast<AF1>(outputWidth), static_cast<AF1>(outputHeight)
			);
			
			consts.Sample[0] = (((m_hdr) && (m_sharpness <= +0.0f)) ? 1 : 0);
			m_constants.fill(&consts);
		}
		
		static const uint32_t threadGroupWorkRegionDim = 16;
		
		uint32_t dispatch[3];
		dispatch[0] = (outputWidth + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;
		dispatch[1] = (outputHeight + (threadGroupWorkRegionDim - 1)) / threadGroupWorkRegionDim;
		dispatch[2] = 1;
		
		m_core.recordMemoryBarrier(cmdStream);
		
		if (m_sharpness > +0.0f) {
			{
				DescriptorWrites writes;
				writes.sampledImageWrites.emplace_back(1, input);
				writes.storageImageWrites.emplace_back(2, m_intermediateImage);
				
				m_core.writeDescriptorSet(m_easuDescriptorSet, writes);
			}
			
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_easuPipeline,
					dispatch,
					{DescriptorSetUsage(0, m_core.getDescriptorSet(
							m_easuDescriptorSet
					).vulkanHandle)},
					PushConstants(0)
			);
			
			{
				FSRConstants consts = {};
				
				FsrRcasCon(consts.Const0, 1.0f / m_sharpness);
				consts.Sample[0] = (m_hdr ? 1 : 0);
				
				m_constants.fill(&consts);
			}
			
			m_core.prepareImageForSampling(cmdStream, m_intermediateImage);
			
			{
				DescriptorWrites writes;
				writes.sampledImageWrites.emplace_back(1, m_intermediateImage);
				writes.storageImageWrites.emplace_back(2, output);
				
				m_core.writeDescriptorSet(m_rcasDescriptorSet, writes);
			}
			
			m_core.recordComputeDispatchToCmdStream(
					cmdStream,
					m_rcasPipeline,
					dispatch,
					{DescriptorSetUsage(0, m_core.getDescriptorSet(
							m_rcasDescriptorSet
					).vulkanHandle)},
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
					m_rcasPipeline,
					dispatch,
					{DescriptorSetUsage(0, m_core.getDescriptorSet(
							m_easuDescriptorSet
					).vulkanHandle)},
					PushConstants(0)
			);
		}
		
		m_core.recordMemoryBarrier(cmdStream);
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
		m_sharpness = sharpness;
	}
	
}
