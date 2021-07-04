#pragma once

#include <vkcv/Event.hpp>

namespace vkcv::shader {
	
	typedef typename event_function<vk::ShaderStageFlagBits, const std::filesystem::path&>::type ShaderCompiledFunction;
	
	class Compiler {
	private:
	public:
		virtual void compile(vk::ShaderStageFlagBits shaderStage, const std::filesystem::path& shaderPath,
							 const ShaderCompiledFunction& compiled, bool update = false) = 0;
		
	};
	
}
