
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
	
}
