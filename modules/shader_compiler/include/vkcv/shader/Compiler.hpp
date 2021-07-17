#pragma once

#include <vkcv/Event.hpp>

namespace vkcv::shader {
	
	typedef typename event_function<ShaderStage, const std::filesystem::path&>::type ShaderCompiledFunction;
	
	class Compiler {
	private:
	public:
		virtual bool compileSource(ShaderStage shaderStage, const char* shaderSource,
								   const ShaderCompiledFunction& compiled,
								   const std::filesystem::path& includePath) = 0;
		
		virtual void compile(ShaderStage shaderStage, const std::filesystem::path& shaderPath,
							 const ShaderCompiledFunction& compiled,
							 const std::filesystem::path& includePath, bool update) = 0;
		
	};
	
}
