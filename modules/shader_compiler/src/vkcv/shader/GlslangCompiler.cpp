
#include "vkcv/shader/GlslangCompiler.hpp"

#include <vkcv/Logger.hpp>

#include <fstream>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace vkcv::shader {
	
	static uint32_t s_CompilerCount = 0;
	
	GlslangCompiler::GlslangCompiler() : Compiler() {
		if (s_CompilerCount == 0) {
			glslang::InitializeProcess();
		}
		
		s_CompilerCount++;
	}
	
	GlslangCompiler::GlslangCompiler(const GlslangCompiler &other) : Compiler(other) {
		s_CompilerCount++;
	}
	
	GlslangCompiler::~GlslangCompiler() {
		s_CompilerCount--;
		
		if (s_CompilerCount == 0) {
			glslang::FinalizeProcess();
		}
	}
	
	GlslangCompiler &GlslangCompiler::operator=(const GlslangCompiler &other) {
		s_CompilerCount++;
		return *this;
	}
	
	bool GlslangCompiler::writeSpirvCode(const std::filesystem::path &shaderPath,
										 const std::vector<uint32_t>& spirv) {
		std::ofstream file (shaderPath.string(), std::ios::out | std::ios::binary);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)", shaderPath.string().c_str());
			return false;
		}
		
		const auto fileSize = static_cast<std::streamsize>(
				sizeof(uint32_t) * spirv.size()
		);
		
		file.seekp(0);
		file.write(reinterpret_cast<const char*>(spirv.data()), fileSize);
		file.close();
		
		return true;
	}
	
	std::vector<char> GlslangCompiler::readShaderCode(const std::filesystem::path &shaderPath) {
		std::ifstream file (shaderPath.string(), std::ios::ate);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)", shaderPath.string().c_str());
			return std::vector<char>{};
		}
		
		std::streamsize fileSize = file.tellg();
		std::vector<char> buffer (fileSize + 1);
		
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		
		buffer[fileSize] = '\0';
		return buffer;
	}
	
	void GlslangCompiler::compile(ShaderStage shaderStage,
								  const std::filesystem::path &shaderPath,
								  const ShaderCompiledFunction &compiled,
								  const std::filesystem::path &includePath,
								  bool update) {
		const std::vector<char> code = readShaderCode(shaderPath);
		bool result;
		
		if (!includePath.empty()) {
			result = compileSource(shaderStage, code.data(), compiled, includePath);
		} else {
			result = compileSource(shaderStage, code.data(), compiled, shaderPath.parent_path());
		}
		
		if (!result) {
			vkcv_log(LogLevel::ERROR, "Shader compilation failed: (%s)", shaderPath.string().c_str());
		}
		
		if (update) {
			// TODO: Shader hot compilation during runtime
		}
	}
	
}
