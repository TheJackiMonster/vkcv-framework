#pragma once

#include <filesystem>
#include <vulkan/vulkan.hpp>
#include "Compiler.hpp"

namespace vkcv::shader {
	
	class GLSLCompiler {
	private:
	public:
		GLSLCompiler();
		
		GLSLCompiler(const GLSLCompiler& other);
		GLSLCompiler(GLSLCompiler&& other) = default;
	
		~GLSLCompiler();
		
		GLSLCompiler& operator=(const GLSLCompiler& other);
		GLSLCompiler& operator=(GLSLCompiler&& other) = default;
		
		void compile(vk::ShaderStageFlagBits shaderStage, const std::filesystem::path& shaderPath,
					 const ShaderCompiledFunction& compiled, bool update = false);
		
	};
	
}
