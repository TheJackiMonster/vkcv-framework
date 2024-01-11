
#include "vkcv/shader/ShadyCompiler.hpp"

#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>

namespace vkcv::shader {
	
	ShadyCompiler::ShadyCompiler() : Compiler() {
		// TODO
	}
	
	ShadyCompiler::ShadyCompiler(const ShadyCompiler &other) : Compiler(other) {
		// TODO
	}
	
	ShadyCompiler::~ShadyCompiler() {
		// TODO
	}
	
	ShadyCompiler &ShadyCompiler::operator=(const ShadyCompiler &other) {
		// TODO
		return *this;
	}
	
	void ShadyCompiler::compile(ShaderStage shaderStage,
								  const std::filesystem::path &shaderPath,
								  const ShaderCompiledFunction &compiled,
								  const std::filesystem::path &includePath,
								  bool update) {
		// TODO
	}
	
}
