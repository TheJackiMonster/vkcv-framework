/**
 * @authors Simeon Hermann
 * @file src/vkcv/ShaderProgram.cpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#include "ShaderProgram.hpp"
#include <fstream>
#include <iostream>



std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


namespace vkcv {

	ShaderProgram::ShaderProgram(vkcv::Context& context)
		: m_context(context) 
	{}

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

	VkShaderModule ShaderProgram::createShaderModule(const std::vector<char>& shaderCode) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_context.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module!");
		}
		return shaderModule;
	}

	VkPipelineShaderStageCreateInfo ShaderProgram::createShaderStage(VkShaderModule& shaderModule, VkShaderStageFlagBits shaderStage) {
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = shaderStage;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		return shaderStageInfo;
	}
	
	ShaderProgram::~ShaderProgram() {
	}

	ShaderProgram ShaderProgram::create(vkcv::Context& context) {
		return ShaderProgram(context);
	}

	void ShaderProgram::addShader(VkShaderStageFlagBits shaderStage, const std::string& filepath) {
		if (containsShaderStage(shaderStage)) {
			throw std::runtime_error("Shader program already contains this particular shader stage.");
		}
		else {
			auto shaderCode = readFile(filepath);
			VkShaderModule shaderModule = createShaderModule(shaderCode);
			VkPipelineShaderStageCreateInfo shaderInfo = createShaderStage(shaderModule, shaderStage);
			m_shaderStagesList.push_back(shaderInfo);
			vkDestroyShaderModule(m_context.getDevice(), shaderModule, nullptr);
		}
	}

	bool ShaderProgram::containsShaderStage(VkShaderStageFlagBits shaderStage) {
		for (int i = 0; i < m_shaderStagesList.size(); i++) {
			if (m_shaderStagesList[i].stage == shaderStage) {
				return true;
			}
		}
		return false;
	}

	bool ShaderProgram::deleteShaderStage(VkShaderStageFlagBits shaderStage) {
		for (int i = 0; i < m_shaderStagesList.size() - 1; i++) {
			if (m_shaderStagesList[i].stage == shaderStage) {
				m_shaderStagesList.erase(m_shaderStagesList.begin() + i);
				return true;
			}
		}
		return false;
	}

	std::vector<VkPipelineShaderStageCreateInfo> ShaderProgram::getShaderStages() {
		return m_shaderStagesList;
	}

	int ShaderProgram::getShaderStagesCount() {
		return m_shaderStagesList.size();
	}
}
