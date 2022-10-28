
#include "vkcv/shader/Compiler.hpp"

namespace vkcv::shader {
	
	void Compiler::compileProgram(ShaderProgram& program,
								  const std::unordered_map<ShaderStage, const std::filesystem::path>& stages,
								  const ShaderProgramCompiledFunction& compiled,
								  const std::filesystem::path& includePath, bool update) {
		std::vector<std::pair<ShaderStage, const std::filesystem::path>> stageList;
		size_t i;
		
		stageList.reserve(stages.size());
		for (const auto& stage : stages) {
			stageList.push_back(stage);
		}
		
		/* Compile a shader programs stages in parallel to improve performance */
		#pragma omp parallel for shared(stageList, includePath, update) private(i)
		for (i = 0; i < stageList.size(); i++) {
			const auto& stage = stageList[i];
			
			compile(
				stage.first,
				stage.second,
				[&program](ShaderStage shaderStage, const std::filesystem::path& path) {
					#pragma omp critical
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
