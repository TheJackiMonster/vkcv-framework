/**
 * @authors Simeon Hermann, Leonie Franken
 * @file src/vkcv/ShaderProgram.cpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#include "vkcv/ShaderProgram.hpp"

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

    vk::ShaderStageFlagBits ShaderProgram::convertToShaderStageFlagBits(ShaderProgram::ShaderStage shaderStage) const{
        switch (shaderStage) {
            case ShaderStage::VERTEX:
                return vk::ShaderStageFlagBits::eVertex;
            case ShaderStage::FRAGMENT:
                return vk::ShaderStageFlagBits::eFragment;
            case ShaderStage::COMPUTE:
                return vk::ShaderStageFlagBits::eCompute;
        }
        throw std::runtime_error("Shader Type not yet implemented.");
	}

	/*vk::ShaderModule ShaderProgram::createShaderModule(const std::vector<char>& shaderCode) {
		vk::ShaderModuleCreateInfo createInfo({}, shaderCode.size(), reinterpret_cast<const uint32_t*>(shaderCode.data()));
		vk::ShaderModule shaderModule;
		if ((m_context.getDevice().createShaderModule(&createInfo, nullptr, &shaderModule)) != vk::Result::eSuccess) {
			throw std::runtime_error("Failed to create shader module!");
		}
		return shaderModule;
	}*/

	/*vk::PipelineShaderStageCreateInfo ShaderProgram::createShaderStage(vk::ShaderModule& shaderModule, vk::ShaderStageFlagBits shaderStage) {
		vk::PipelineShaderStageCreateInfo shaderStageInfo({}, shaderStage, shaderModule, "main", {});
		shaderStageInfo.stage = shaderStage;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		return shaderStageInfo;
	}*/
	
	ShaderProgram::~ShaderProgram() {
	}

	ShaderProgram ShaderProgram::create() {
		return ShaderProgram();
	}

	void ShaderProgram::addShader(ShaderProgram::ShaderStage shaderStage, const std::string& filepath) {
		if (containsShaderStage(shaderStage)) {
			throw std::runtime_error("Shader program already contains this particular shader stage.");
		}
		else {
			auto shaderCode = readFile(filepath);
            vk::ShaderStageFlagBits convertedShaderStage = convertToShaderStageFlagBits(shaderStage);
			//vk::ShaderModule shaderModule = createShaderModule(shaderCode);
			//vk::PipelineShaderStageCreateInfo shaderInfo = createShaderStage(shaderModule, shaderStage);
			//m_shaderStagesList.push_back(shaderInfo);
			//m_context.getDevice().destroyShaderModule(shaderModule, nullptr);
			m_shaderStages.shaderCode.push_back(shaderCode);
			m_shaderStages.shaderStageFlag.push_back(convertedShaderStage);
		}
	}

	bool ShaderProgram::containsShaderStage(ShaderProgram::ShaderStage shaderStage) const{
        vk::ShaderStageFlagBits convertedShaderStage = convertToShaderStageFlagBits(shaderStage);
		for (int i = 0; i < m_shaderStages.shaderStageFlag.size(); i++) {
			if (m_shaderStages.shaderStageFlag[i] == convertedShaderStage) {
				return true;
			}
		}
		return false;
	}

	bool ShaderProgram::deleteShaderStage(ShaderProgram::ShaderStage shaderStage) {
        vk::ShaderStageFlagBits convertedShaderStage = convertToShaderStageFlagBits(shaderStage);
		for (int i = 0; i < m_shaderStages.shaderStageFlag.size() - 1; i++) {
			if (m_shaderStages.shaderStageFlag[i] == convertedShaderStage) {
			    m_shaderStages.shaderStageFlag.erase(m_shaderStages.shaderStageFlag.begin() + i);
                m_shaderStages.shaderCode.erase(m_shaderStages.shaderCode.begin() + i);
				return true;
			}
		}
		return false;
	}

	std::vector<vk::ShaderStageFlagBits> ShaderProgram::getShaderStages() const{
		return m_shaderStages.shaderStageFlag;
	}

    std::vector<std::vector<char>> ShaderProgram::getShaderCode() const {
	    return m_shaderStages.shaderCode;
	}

	int ShaderProgram::getShaderStagesCount() const {
		return m_shaderStages.shaderStageFlag.size();
	}
}
