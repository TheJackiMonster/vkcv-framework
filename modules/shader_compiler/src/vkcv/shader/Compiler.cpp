
#include "vkcv/shader/Compiler.hpp"

namespace vkcv::shader {
	
	void Compiler::compileProgram(ShaderProgram& program,
								  const std::unordered_map<ShaderStage, const std::filesystem::path>& stages,
								  const ShaderProgramCompiledFunction& compiled,
								  const std::filesystem::path& includePath, bool update) {
		for (const auto& stage : stages) {
			compile(
				stage.first,
				stage.second,
				[&program](ShaderStage shaderStage, const std::filesystem::path& path) {
					program.addShader(shaderStage, path);
				},
				includePath,
				update
			);
		}
		
		if (compiled) {
			compiled(program);
		}
	}
	
	std::string Compiler::getDefine(const std::string &name) const {
		return m_defines.at(name);
	}
	
	void Compiler::setDefine(const std::string &name, const std::string &value) {
		m_defines[name] = value;
	}
	
}
