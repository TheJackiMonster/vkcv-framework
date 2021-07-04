/**
 * @authors Simeon Hermann, Leonie Franken
 * @file src/vkcv/ShaderProgram.cpp
 * @brief ShaderProgram class to handle and prepare the shader stages for a graphics pipeline
 */

#include "vkcv/ShaderProgram.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv {
    /**
     * Reads the file of a given shader code.
     * Only used within the class.
     * @param[in] relative path to the shader code
     * @return vector of chars as a buffer for the code
     */
	std::vector<char> readShaderCode(const std::filesystem::path &shaderPath) {
		std::ifstream file (shaderPath.string(), std::ios::ate | std::ios::binary);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened");
			return std::vector<char>{};
		}
		
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		
        return buffer;
	}

	VertexAttachmentFormat convertFormat(spirv_cross::SPIRType::BaseType basetype, uint32_t vecsize){
        switch (basetype) {
            case spirv_cross::SPIRType::Int:
                switch (vecsize) {
                    case 1:
                        return VertexAttachmentFormat::INT;
                    case 2:
                        return VertexAttachmentFormat::INT2;
                    case 3:
                        return VertexAttachmentFormat::INT3;
                    case 4:
                        return VertexAttachmentFormat::INT4;
                    default:
                        break;
                }
                break;
            case spirv_cross::SPIRType::Float:
                switch (vecsize) {
                    case 1:
                        return VertexAttachmentFormat::FLOAT;
                    case 2:
                        return VertexAttachmentFormat::FLOAT2;
                    case 3:
                        return VertexAttachmentFormat::FLOAT3;
                    case 4:
                        return VertexAttachmentFormat::FLOAT4;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
		
		vkcv_log(LogLevel::WARNING, "Unknown vertex format");
        return VertexAttachmentFormat::FLOAT;
	}

	ShaderProgram::ShaderProgram() noexcept :
	m_Shaders{},
    m_VertexAttachments{},
    m_DescriptorSets{}
	{}

	bool ShaderProgram::addShader(vk::ShaderStageFlagBits shaderStage, const std::filesystem::path &shaderPath)
	{
	    if(m_Shaders.find(shaderStage) != m_Shaders.end()) {
			vkcv_log(LogLevel::WARNING, "Overwriting existing shader stage");
		}

	    const std::vector<char> shaderCode = readShaderCode(shaderPath);
	    
	    if (shaderCode.empty()) {
			return false;
		} else {
            Shader shader{shaderCode, shaderStage};
            m_Shaders.insert(std::make_pair(shaderStage, shader));
            reflectShader(shaderStage);
            return true;
        }
	}

    const Shader &ShaderProgram::getShader(vk::ShaderStageFlagBits shaderStage) const
    {
	    return m_Shaders.at(shaderStage);
	}

    bool ShaderProgram::existsShader(vk::ShaderStageFlagBits shaderStage) const
    {
	    if(m_Shaders.find(shaderStage) == m_Shaders.end())
	        return false;
	    else
	        return true;
    }

    void ShaderProgram::reflectShader(vk::ShaderStageFlagBits shaderStage)
    {
        auto shaderCodeChar = m_Shaders.at(shaderStage).shaderCode;
        std::vector<uint32_t> shaderCode;

        for (uint32_t i = 0; i < shaderCodeChar.size()/4; i++)
            shaderCode.push_back(((uint32_t*) shaderCodeChar.data())[i]);

        spirv_cross::Compiler comp(move(shaderCode));
        spirv_cross::ShaderResources resources = comp.get_shader_resources();

        //reflect vertex input
		if (shaderStage == vk::ShaderStageFlagBits::eVertex)
		{
			// spirv-cross API (hopefully) returns the stage_inputs in order
			for (uint32_t i = 0; i < resources.stage_inputs.size(); i++)
			{
                // spirv-cross specific objects
				auto& stage_input = resources.stage_inputs[i];
				const spirv_cross::SPIRType& base_type = comp.get_type(stage_input.base_type_id);

				// vertex input location
				const uint32_t attachment_loc = comp.get_decoration(stage_input.id, spv::DecorationLocation);
                // vertex input name
                const std::string attachment_name = stage_input.name;
				// vertex input format (implies its size)
				const VertexAttachmentFormat attachment_format = convertFormat(base_type.basetype, base_type.vecsize);

                m_VertexAttachments.emplace_back(attachment_loc, attachment_name, attachment_format);
            }
		}

		//reflect descriptor sets (uniform buffer, storage buffer, sampler, sampled image, storage image)
        // std::vector<std::pair<uint32_t, DescriptorBinding>> bindings;
        int32_t maxSetID = -1;
        for (uint32_t i = 0; i < resources.uniform_buffers.size(); i++) {
            auto& u = resources.uniform_buffers[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);

            const uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            const uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            const DescriptorBinding binding(bindingID, DescriptorType::UNIFORM_BUFFER, base_type.vecsize, shaderStage);

            auto setIter = m_DescriptorSets.find(setID);
            if (setIter == m_DescriptorSets.end())
            {
                // create a map for this set ID
                std::unordered_map<uint32_t, DescriptorBinding> setBindings;
                // insert the binding to this set ID's bindings
                setBindings.insert(std::make_pair(bindingID, binding));
                // insert this set ID's map to the descriptor sets
                m_DescriptorSets.insert(std::make_pair(setID, setBindings));
            }
            else
            {
                // search for an existing binding for this set
                auto bindingIter = setIter->second.find(bindingID);
                if (bindingIter == setIter->second.end())
                {
                    // if binding did not exist, insert it
                    setIter->second.insert(std::make_pair(bindingID, binding));
                }
                else
                {
                    if(bindingIter->second.descriptorType != DescriptorType::UNIFORM_BUFFER)
                        vkcv_log(LogLevel::WARNING, "Descriptor type mismatch in shader reflection!");
                    // if binding exists, append additional shader stage to it
                    bindingIter->second.shaderStages | shaderStage;
                }
            }
        }

        for (uint32_t i = 0; i < resources.storage_buffers.size(); i++) {
            auto& u = resources.storage_buffers[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);

            const uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            const uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            const DescriptorBinding binding(bindingID, DescriptorType::STORAGE_BUFFER, base_type.vecsize, shaderStage);

            auto setIter = m_DescriptorSets.find(setID);
            if (setIter == m_DescriptorSets.end())
            {
                // create a map for this set ID
                std::unordered_map<uint32_t, DescriptorBinding> setBindings;
                // insert the binding to this set ID's bindings
                setBindings.insert(std::make_pair(bindingID, binding));
                // insert this set ID's map to the descriptor sets
                m_DescriptorSets.insert(std::make_pair(setID, setBindings));
            }
            else
            {
                // search for an existing binding for this set
                auto bindingIter = setIter->second.find(bindingID);
                if (bindingIter == setIter->second.end())
                {
                    // if binding did not exist, insert it
                    setIter->second.insert(std::make_pair(bindingID, binding));
                }
                else
                {
                    if (bindingIter->second.descriptorType != DescriptorType::STORAGE_BUFFER)
                        vkcv_log(LogLevel::WARNING, "Descriptor type mismatch in shader reflection!");
                    // if binding exists, append additional shader stage to it
                    bindingIter->second.shaderStages | shaderStage;
                }
            }
        }

        for (uint32_t i = 0; i < resources.separate_samplers.size(); i++) {
            auto& u = resources.separate_samplers[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);

            const uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            const uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            const DescriptorBinding binding(bindingID, DescriptorType::SAMPLER, base_type.vecsize, shaderStage);

            auto setIter = m_DescriptorSets.find(setID);
            if (setIter == m_DescriptorSets.end())
            {
                // create a map for this set ID
                std::unordered_map<uint32_t, DescriptorBinding> setBindings;
                // insert the binding to this set ID's bindings
                setBindings.insert(std::make_pair(bindingID, binding));
                // insert this set ID's map to the descriptor sets
                m_DescriptorSets.insert(std::make_pair(setID, setBindings));
            }
            else
            {
                // search for an existing binding for this set
                auto bindingIter = setIter->second.find(bindingID);
                if (bindingIter == setIter->second.end())
                {
                    // if binding did not exist, insert it
                    setIter->second.insert(std::make_pair(bindingID, binding));
                }
                else
                {
                    if (bindingIter->second.descriptorType != DescriptorType::SAMPLER)
                        vkcv_log(LogLevel::WARNING, "Descriptor type mismatch in shader reflection!");
                    // if binding exists, append additional shader stage to it
                    bindingIter->second.shaderStages | shaderStage;
                }
            }
        }

        for (uint32_t i = 0; i < resources.separate_images.size(); i++) {
            auto& u = resources.separate_images[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);

            const uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            const uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            const DescriptorBinding binding(bindingID, DescriptorType::IMAGE_SAMPLED, base_type.vecsize, shaderStage);

            auto setIter = m_DescriptorSets.find(setID);
            if (setIter == m_DescriptorSets.end())
            {
                // create a map for this set ID
                std::unordered_map<uint32_t, DescriptorBinding> setBindings;
                // insert the binding to this set ID's bindings
                setBindings.insert(std::make_pair(bindingID, binding));
                // insert this set ID's map to the descriptor sets
                m_DescriptorSets.insert(std::make_pair(setID, setBindings));
            }
            else
            {
                // search for an existing binding for this set
                auto bindingIter = setIter->second.find(bindingID);
                if (bindingIter == setIter->second.end())
                {
                    // if binding did not exist, insert it
                    setIter->second.insert(std::make_pair(bindingID, binding));
                }
                else
                {
                    if (bindingIter->second.descriptorType != DescriptorType::IMAGE_SAMPLED)
                        vkcv_log(LogLevel::WARNING, "Descriptor type mismatch in shader reflection!");
                    // if binding exists, append additional shader stage to it
                    bindingIter->second.shaderStages | shaderStage;
                }
            }
        }

        for (uint32_t i = 0; i < resources.storage_images.size(); i++) {
            auto& u = resources.storage_images[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);

            const uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            const uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            const DescriptorBinding binding(bindingID, DescriptorType::IMAGE_STORAGE, base_type.vecsize, shaderStage);

            auto setIter = m_DescriptorSets.find(setID);
            if (setIter == m_DescriptorSets.end())
            {
                // create a map for this set ID
                std::unordered_map<uint32_t, DescriptorBinding> setBindings;
                // insert the binding to this set ID's bindings
                setBindings.insert(std::make_pair(bindingID, binding));
                // insert this set ID's map to the descriptor sets
                m_DescriptorSets.insert(std::make_pair(setID, setBindings));
            }
            else
            {
                // search for an existing binding for this set
                auto bindingIter = setIter->second.find(bindingID);
                if (bindingIter == setIter->second.end())
                {
                    // if binding did not exist, insert it
                    setIter->second.insert(std::make_pair(bindingID, binding));
                }
                else
                {
                    if (bindingIter->second.descriptorType != DescriptorType::IMAGE_STORAGE)
                        vkcv_log(LogLevel::WARNING, "Descriptor type mismatch in shader reflection!");
                    // if binding exists, append additional shader stage to it
                    bindingIter->second.shaderStages | shaderStage;
                }
            }
        }

        //reflect push constants
		for (const auto &pushConstantBuffer : resources.push_constant_buffers) {
			for (const auto &range : comp.get_active_buffer_ranges(pushConstantBuffer.id)) {
				const size_t size = range.range + range.offset;
				m_pushConstantSize = std::max(m_pushConstantSize, size);
			}
		}
    }

    const std::vector<VertexAttachment> &ShaderProgram::getVertexAttachments() const
    {
        return m_VertexAttachments;
	}

    const std::unordered_map<uint32_t, std::unordered_map<uint32_t, DescriptorBinding>>& ShaderProgram::getReflectedDescriptors() const {
        return m_DescriptorSets;
    }

	size_t ShaderProgram::getPushConstantSize() const
	{
		return m_pushConstantSize;
	}
}
