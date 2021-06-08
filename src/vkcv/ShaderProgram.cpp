/**
 * @authors Simeon Hermann, Leonie Franken
 * @file src/vkcv/ShaderProgram.cpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#include "vkcv/ShaderProgram.hpp"
#include <algorithm>

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

	VertexFormat convertFormat(spirv_cross::SPIRType::BaseType basetype, uint32_t vecsize){
        switch (basetype) {
            case spirv_cross::SPIRType::Int:
                switch (vecsize) {
                    case 1:
                        return VertexFormat::INT;
                    case 2:
                        return VertexFormat::INT2;
                    case 3:
                        return VertexFormat::INT3;
                    case 4:
                        return VertexFormat::INT4;
                    default:
                        break;
                }
                break;
            case spirv_cross::SPIRType::Float:
                switch (vecsize) {
                    case 1:
                        return VertexFormat::FLOAT;
                    case 2:
                        return VertexFormat::FLOAT2;
                    case 3:
                        return VertexFormat::FLOAT3;
                    case 4:
                        return VertexFormat::FLOAT4;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        std::cout << "Shader Program Reflection: unknown Vertex Format" << std::endl;
        return VertexFormat::FLOAT;
	}

	ShaderProgram::ShaderProgram() noexcept :
	m_Shaders{},
    m_VertexLayout{},
    m_DescriptorSetLayout{}
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

    void ShaderProgram::reflectShader(ShaderStage shaderStage)
    {
        auto shaderCodeChar = m_Shaders.at(shaderStage).shaderCode;
        std::vector<uint32_t> shaderCode;

        for (uint32_t i = 0; i < shaderCodeChar.size()/4; i++) {
            shaderCode.push_back(((uint32_t*) shaderCodeChar.data())[i]);
        }

        spirv_cross::Compiler comp(move(shaderCode));
        spirv_cross::ShaderResources resources = comp.get_shader_resources();

		if (shaderStage == ShaderStage::VERTEX) {
			std::vector<VertexInputAttachment> inputVec;
			uint32_t offset = 0;

			for (uint32_t i = 0; i < resources.stage_inputs.size(); i++) {
				auto& u = resources.stage_inputs[i];
				const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);
				VertexInputAttachment input = VertexInputAttachment(comp.get_decoration(u.id, spv::DecorationLocation),
					0,
                    u.name,
					convertFormat(base_type.basetype, base_type.vecsize),
					offset);
				inputVec.push_back(input);
				offset += base_type.vecsize * base_type.width / 8;
			}
			m_VertexLayout = VertexLayout(inputVec);
		}

		//Descriptor Sets
		//Storage buffer, uniform Buffer, storage image, sampled image, sampler (?)

        std::vector<uint32_t> separateImageVec;
        for (uint32_t i = 0; i < resources.separate_images.size(); i++) {
            auto &u = resources.separate_images[i];
            separateImageVec.push_back(comp.get_decoration(u.id, spv::DecorationDescriptorSet));
        }

        std::vector<uint32_t> storageImageVec;
        for (uint32_t i = 0; i < resources.storage_images.size(); i++) {
            auto &u = resources.storage_images[i];
            storageImageVec.push_back(comp.get_decoration(u.id, spv::DecorationDescriptorSet));
        }

        std::vector<uint32_t> uniformBufferVec;
        for (uint32_t i = 0; i < resources.uniform_buffers.size(); i++) {
            auto &u = resources.uniform_buffers[i];
            uniformBufferVec.push_back(comp.get_decoration(u.id, spv::DecorationDescriptorSet));
        }

		std::vector<uint32_t> storageBufferVec;
        for (uint32_t i = 0; i < resources.storage_buffers.size(); i++) {
            auto &u = resources.storage_buffers[i];
            storageBufferVec.push_back(comp.get_decoration(u.id, spv::DecorationDescriptorSet));
        }

        std::vector<uint32_t> samplerVec;
        for (uint32_t i = 0; i < resources.separate_samplers.size(); i++) {
            auto &u = resources.separate_samplers[i];
            samplerVec.push_back(comp.get_decoration(u.id, spv::DecorationDescriptorSet));
        }

        m_DescriptorSetLayout = DescriptorSetLayout(separateImageVec, storageImageVec, uniformBufferVec, storageBufferVec, samplerVec);

		for (const auto &pushConstantBuffer : resources.push_constant_buffers) {
			for (const auto &range : comp.get_active_buffer_ranges(pushConstantBuffer.id)) {
				const size_t size = range.range + range.offset;
				m_pushConstantSize = std::max(m_pushConstantSize, size);
			}
		}
    }

    const VertexLayout& ShaderProgram::getVertexLayout() const{
        return m_VertexLayout;
	}

    const DescriptorSetLayout& ShaderProgram::getDescriptorSetLayout() const {
        return m_DescriptorSetLayout;
    }
	size_t ShaderProgram::getPushConstantSize() const {
		return m_pushConstantSize;
	}
}
