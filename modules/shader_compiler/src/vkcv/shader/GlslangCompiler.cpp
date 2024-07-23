
#include "vkcv/shader/GlslangCompiler.hpp"

#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/glslang/Public/ShaderLang.h>

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
	
	void GlslangCompiler::compile(ShaderStage shaderStage,
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
