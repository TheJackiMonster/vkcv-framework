/**
 * @authors Simeon Hermann, Leonie Franken
 * @file src/vkcv/ShaderProgram.cpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#include "vkcv/ShaderProgram.hpp"

namespace vkcv {
    /**
     * Reads the file of a given shader code.
     * Only used within the class.
     * @param[in] relative path to the shader code
     * @return vector of chars as a buffer for the code
     */
	std::vector<char> readShaderCode(const std::filesystem::path &shaderPath)
	{
		std::ifstream file(shaderPath.string(), std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
		    std::cout << "The file could not be opened." << std::endl;
			return std::vector<char>{};
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
        return buffer;
	}

	ShaderProgram::ShaderProgram() noexcept :
	m_Shaders{}
	{}

	bool ShaderProgram::addShader(ShaderStage shaderStage, const std::filesystem::path &shaderPath)
	{
	    if(m_Shaders.find(shaderStage) != m_Shaders.end())
	        std::cout << "Found existing shader stage. Overwriting."  << std::endl;

	    const std::vector<char> shaderCode = readShaderCode(shaderPath);
	    if (shaderCode.empty())
	        return false;
	    else
        {
            Shader shader{shaderCode, shaderStage};
            m_Shaders.insert(std::make_pair(shaderStage, shader));
            return true;
        }
	}

    const Shader &ShaderProgram::getShader(ShaderStage shaderStage) const
    {
	    return m_Shaders.at(shaderStage);
	}

    bool ShaderProgram::existsShader(ShaderStage shaderStage) const
    {
	    if(m_Shaders.find(shaderStage) == m_Shaders.end())
	        return false;
	    else
	        return true;
    }

    void ShaderProgram::reflectShader(ShaderStage shaderStage) const
    {
        auto shaderCode = m_Shaders.at(shaderStage).shaderCode;
        //TODO
    }
}
