#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <vkcv/Event.hpp>
#include <vkcv/ShaderStage.hpp>
#include <vkcv/ShaderProgram.hpp>

namespace vkcv::shader {
	
	typedef typename event_function<ShaderStage, const std::filesystem::path&>::type ShaderCompiledFunction;
	typedef typename event_function<ShaderProgram&>::type ShaderProgramCompiledFunction;
	
	class Compiler {
	private:
	protected:
		std::unordered_map<std::string, std::string> m_defines;
		
	public:
		virtual bool compileSource(ShaderStage shaderStage, const char* shaderSource,
								   const ShaderCompiledFunction& compiled,
								   const std::filesystem::path& includePath) = 0;
		
		virtual void compile(ShaderStage shaderStage, const std::filesystem::path& shaderPath,
							 const ShaderCompiledFunction& compiled,
							 const std::filesystem::path& includePath, bool update) = 0;
		
		void compileProgram(ShaderProgram& program,
							const std::unordered_map<ShaderStage, const std::filesystem::path>& stages,
							const ShaderProgramCompiledFunction& compiled,
							const std::filesystem::path& includePath = "", bool update = false);
		
		std::string getDefine(const std::string& name) const;
		
		void setDefine(const std::string& name, const std::string& value);
	};
	
}
