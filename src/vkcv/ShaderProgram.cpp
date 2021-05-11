/**
 * @authors Simeon Hermann, Leonie Franken
 * @file src/vkcv/ShaderProgram.cpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#include "ShaderProgram.hpp"

std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


namespace vkcv {

	ShaderProgram::ShaderProgram(){
	    ShaderStages m_shaderStages{};
	    m_shaderStages.shaderCode = std::vector<std::vector<char>> ();
	    m_shaderStages.shaderStageFlag = std::vector<vk::ShaderStageFlagBits> ();
	}

	std::vector<char> ShaderProgram::readFile(const std::string& filepath) {
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("The file could not be opened.");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		return buffer;
	}

	vk::ShaderModule ShaderProgram::createShaderModule(const std::vector<char>& shaderCode) {
		vk::ShaderModuleCreateInfo createInfo({}, shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()));
		vk::ShaderModule shaderModule;
		if ((m_context.getDevice().createShaderModule(&createInfo, nullptr, &shaderModule)) != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to create shader module!");
		}
		return shaderModule;
	}

	vk::PipelineShaderStageCreateInfo ShaderProgram::createShaderStage(vk::ShaderModule& shaderModule, vk::ShaderStageFlagBits shaderStage) {
		vk::PipelineShaderStageCreateInfo shaderStageInfo({}, shaderStage, shaderModule, "main", {});
		shaderStageInfo.stage = shaderStage;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		return shaderStageInfo;
	}
	
	ShaderProgram::~ShaderProgram() {
	}

	ShaderProgram ShaderProgram::create() {
		return ShaderProgram();
	}

	//TODO: Enum übergeben statt ShaderStageFlagBits
	void ShaderProgram::addShader(vk::ShaderStageFlagBits shaderStage, const std::string& filepath) {
		if (containsShaderStage(shaderStage)) {
			throw std::runtime_error("Shader program already contains this particular shader stage.");
		}
		else {
			auto shaderCode = readFile(filepath);
			//vk::ShaderModule shaderModule = createShaderModule(shaderCode);
			//vk::PipelineShaderStageCreateInfo shaderInfo = createShaderStage(shaderModule, shaderStage);
			//m_shaderStagesList.push_back(shaderInfo);
			//m_context.getDevice().destroyShaderModule(shaderModule, nullptr);
			m_shaderStages.shaderCode.push_back(shaderCode);
			m_shaderStages.shaderStageFlag.push_back(shaderStage);

		}
	}

	bool ShaderProgram::containsShaderStage(vk::ShaderStageFlagBits shaderStage) {
		for (int i = 0; i < m_shaderStagesList.size(); i++) {
			if (m_shaderStagesList[i].stage == shaderStage) {
				return true;
			}
		}
		return false;
	}

	bool ShaderProgram::deleteShaderStage(vk::ShaderStageFlagBits shaderStage) {
		for (int i = 0; i < m_shaderStagesList.size() - 1; i++) {
			if (m_shaderStagesList[i].stage == shaderStage) {
				m_shaderStagesList.erase(m_shaderStagesList.begin() + i);
				return true;
			}
		}
		return false;
	}

	std::vector<vk::PipelineShaderStageCreateInfo> ShaderProgram::getShaderStages() {
		return m_shaderStagesList;
	}

	int ShaderProgram::getShaderStagesCount() {
		return m_shaderStagesList.size();
	}
}
