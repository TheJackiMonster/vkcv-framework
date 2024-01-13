
#include "vkcv/shader/ShadyCompiler.hpp"

#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>

namespace vkcv::shader {
	
	ShadyCompiler::ShadyCompiler()
	: Compiler() {}
	
	void ShadyCompiler::compile(ShaderStage shaderStage,
								const std::filesystem::path &shaderPath,
								const ShaderCompiledFunction &compiled,
								const std::filesystem::path &includePath,
								bool update) {
		std::string shaderCode;
		bool result = readTextFromFile(shaderPath, shaderCode);
		
		if (!result) {
			vkcv_log(LogLevel::ERROR, "Loading shader failed: (%s)", shaderPath.string().c_str());
		}
		
		if (!includePath.empty()) {
			result = compileSource(shaderStage, shaderCode, compiled, includePath);
		} else {
			result = compileSource(shaderStage, shaderCode, compiled, shaderPath.parent_path());
		}
		
		if (!result) {
			vkcv_log(LogLevel::ERROR, "Shader compilation failed: (%s)", shaderPath.string().c_str());
		}
		
		if (update) {
			// TODO: Shader hot compilation during runtime
		}
	}
	
}
