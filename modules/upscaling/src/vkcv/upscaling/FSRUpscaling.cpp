
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
	
	static bool compileFSRShaders(const shader::ShaderCompiledFunction& compiled) {
		std::filesystem::path directory = generateTemporaryFilePath();
		
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
		
		vkcv::shader::GLSLCompiler compiler;
		return compiler.compileSource(vkcv::ShaderStage::COMPUTE,
									  FSR_PASS_GLSL_SHADER.c_str(),
									  compiled, directory);
	}
	
	FSRUpscaling::FSRUpscaling() {
		compileFSRShaders([&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
			m_program.addShader(shaderStage, path);
		});
	}

}
