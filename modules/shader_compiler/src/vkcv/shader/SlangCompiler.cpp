
#include "vkcv/shader/SlangCompiler.hpp"

#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>

#include <slang.h>

namespace vkcv::shader {
	
	static uint32_t s_CompilerCount = 0;
  static slang::IGlobalSession* s_GlobalSession = nullptr;
	
	SlangCompiler::SlangCompiler() : Compiler() {
		if (s_CompilerCount == 0) {
      slang::createGlobalSession(&s_GlobalSession);
		}
		
		s_CompilerCount++;
	}
	
	SlangCompiler::SlangCompiler(const SlangCompiler &other) : Compiler(other) {
		s_CompilerCount++;
	}
	
	SlangCompiler::~SlangCompiler() {
		s_CompilerCount--;

    if ((s_CompilerCount == 0) && (s_GlobalSession != nullptr)) {
      spDestroySession(s_GlobalSession);
      s_GlobalSession = nullptr;
    }
	}
	
	SlangCompiler &SlangCompiler::operator=(const SlangCompiler &other) {
		s_CompilerCount++;
		return *this;
	}
	
	void SlangCompiler::compile(ShaderStage shaderStage,
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
