#pragma once

#include <filesystem>

#include <vkcv/ShaderStage.hpp>
#include "Compiler.hpp"

namespace vkcv::shader {
	
	class GLSLCompiler : Compiler {
	private:
	public:
		GLSLCompiler();
		
		GLSLCompiler(const GLSLCompiler& other);
		GLSLCompiler(GLSLCompiler&& other) = default;
	
		~GLSLCompiler();
		
		GLSLCompiler& operator=(const GLSLCompiler& other);
		GLSLCompiler& operator=(GLSLCompiler&& other) = default;
		
		bool compileSource(ShaderStage shaderStage, const char* shaderSource,
						   const ShaderCompiledFunction& compiled,
						   const std::filesystem::path& includePath);
		
		void compile(ShaderStage shaderStage, const std::filesystem::path& shaderPath,
					 const ShaderCompiledFunction& compiled,
					 const std::filesystem::path& includePath = "", bool update = false) override;
		
	};
	
}
