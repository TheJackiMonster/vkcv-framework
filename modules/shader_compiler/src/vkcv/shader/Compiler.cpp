
#include "vkcv/shader/Compiler.hpp"

#include <vkcv/File.hpp>
#include <vkcv/Logger.hpp>

namespace vkcv::shader {
	
	std::string Compiler::processShaderSource(const std::string &shaderSource) {
		return shaderSource;
	}
	
	bool Compiler::compileSourceWithHeaders(ShaderStage shaderStage,
											const std::string &shaderSource,
											const std::unordered_map<std::filesystem::path, std::string> &shaderHeaders,
											const ShaderCompiledFunction &compiled) {
		const std::filesystem::path directory = generateTemporaryDirectoryPath();
		
		if (!std::filesystem::create_directory(directory)) {
			vkcv_log(LogLevel::ERROR, "The directory could not be created (%s)", directory.c_str());
			return false;
		}
		
		for (const auto& header : shaderHeaders) {
			if (header.first.has_parent_path()) {
				std::filesystem::create_directories(directory / header.first.parent_path());
			}
			
			if (!writeTextToFile(directory / header.first, processShaderSource(header.second))) {
				return false;
			}
		}
		
		return compileSource(
				shaderStage,
				shaderSource,
				[&directory, &compiled] (vkcv::ShaderStage shaderStage,
										 const std::filesystem::path& path) {
					if (compiled) {
						compiled(shaderStage, path);
					}
					
					std::filesystem::remove_all(directory);
				}, directory
		);
	}
	
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
