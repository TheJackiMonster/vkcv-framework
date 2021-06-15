#pragma once

#include <vkcv/Event.hpp>

namespace vkcv::shader {
	
	typedef typename event_function<ShaderStage, const std::filesystem::path&>::type ShaderCompiledFunction;
	
	class Compiler {
	private:
	public:
		virtual void compile(ShaderStage shaderStage, const std::filesystem::path& shaderPath,
							 const ShaderCompiledFunction& compiled, bool update = false) = 0;
		
	};
	
}
