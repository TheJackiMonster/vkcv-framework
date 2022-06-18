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
	std::vector<uint32_t> readShaderCode(const std::filesystem::path &shaderPath) {
		std::ifstream file (shaderPath.string(), std::ios::ate | std::ios::binary);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened: %s", shaderPath.c_str());
			return std::vector<uint32_t>();
		}
		
		size_t fileSize = (size_t)file.tellg();

        if (fileSize % sizeof(uint32_t) != 0) {
            vkcv_log(LogLevel::ERROR, "The file is not a valid shader: %s", shaderPath.c_str());
            return std::vector<uint32_t>();
        }

		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		
		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();
		
        return buffer;
	}

	ShaderProgram::ShaderProgram() noexcept :
	m_Shaders{},
    m_DescriptorSets{}
	{}

	bool ShaderProgram::addShader(ShaderStage stage, const std::filesystem::path &path)
	{
	    if(m_Shaders.find(stage) != m_Shaders.end()) {
			vkcv_log(LogLevel::WARNING, "Overwriting existing shader stage");
		}

	    const std::vector<uint32_t> shaderCode = readShaderCode(path);
	    
	    if (shaderCode.empty()) {
			return false;
		} else {
            m_Shaders.insert(std::make_pair(stage, shaderCode));
            reflectShader(stage);
            return true;
        }
	}

    const std::vector<uint32_t> &ShaderProgram::getShaderBinary(ShaderStage stage) const
    {
	    return m_Shaders.at(stage);
	}

    bool ShaderProgram::existsShader(ShaderStage stage) const
    {
	    if(m_Shaders.find(stage) == m_Shaders.end())
	        return false;
	    else
	        return true;
    }

    void ShaderProgram::reflectShader(ShaderStage shaderStage)
    {
        auto shaderCode = m_Shaders.at(shaderStage);

        spirv_cross::Compiler comp(shaderCode);
        spirv_cross::ShaderResources resources = comp.get_shader_resources();

        //reflect vertex input
		if (shaderStage == ShaderStage::VERTEX)
		{
			if (!resources.stage_inputs.empty()) {
				vkcv_log(LogLevel::WARNING, "Vertex bindings are not supported");
			}
		}

		//reflect descriptor sets (uniform buffer, storage buffer, sampler, sampled image, storage image)
        std::vector<std::pair<uint32_t, DescriptorBinding>> bindings;

        for (uint32_t i = 0; i < resources.uniform_buffers.size(); i++)
        {
            auto& u = resources.uniform_buffers[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);
            const spirv_cross::SPIRType &type      = comp.get_type(u.type_id);

            uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            uint32_t descriptorCount = base_type.vecsize;
            bool variableCount = false;
            // query whether reflected resources are qualified as one-dimensional array
            if(type.array_size_literal[0])
            {
                descriptorCount = type.array[0];
                if(type.array[0] == 0)
                    variableCount = true;
            }

            DescriptorBinding binding {
				bindingID,
				DescriptorType::UNIFORM_BUFFER,
				descriptorCount,
				shaderStage,
				variableCount,
				variableCount // partialBinding == variableCount
			};

            auto insertionResult = m_DescriptorSets[setID].insert(std::make_pair(bindingID, binding));
            if(!insertionResult.second)
            {
                vkcv_log(LogLevel::WARNING,
                         "Attempting to overwrite already existing binding %u at set ID %u.",
                         bindingID,
                         setID);
            }
        }

        for (uint32_t i = 0; i < resources.storage_buffers.size(); i++)
        {
            auto& u = resources.storage_buffers[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);
            const spirv_cross::SPIRType &type      = comp.get_type(u.type_id);

            uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            uint32_t descriptorCount = base_type.vecsize;
            bool variableCount = false;
            // query whether reflected resources are qualified as one-dimensional array
            if(type.array_size_literal[0])
            {
                descriptorCount = type.array[0];
                if(type.array[0] == 0)
                    variableCount = true;
            }

            DescriptorBinding binding {
				bindingID,
				DescriptorType::STORAGE_BUFFER,
				descriptorCount,
				shaderStage,
				variableCount,
				variableCount // partialBinding == variableCount
			};

            auto insertionResult = m_DescriptorSets[setID].insert(std::make_pair(bindingID, binding));
            if(!insertionResult.second)
            {
                vkcv_log(LogLevel::WARNING,
                         "Attempting to overwrite already existing binding %u at set ID %u.",
                         bindingID,
                         setID);
            }
        }

        for (uint32_t i = 0; i < resources.separate_samplers.size(); i++) {
            auto& u = resources.separate_samplers[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);
            const spirv_cross::SPIRType &type      = comp.get_type(u.type_id);

            uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            uint32_t descriptorCount = base_type.vecsize;
            bool variableCount = false;
            // query whether reflected resources are qualified as one-dimensional array
            if(type.array_size_literal[0])
            {
                descriptorCount = type.array[0];
                if(type.array[0] == 0)
                    variableCount = true;
            }

            DescriptorBinding binding {
				bindingID,
				DescriptorType::SAMPLER,
				descriptorCount,
				shaderStage,
				variableCount,
				variableCount // partialBinding == variableCount
			};

            auto insertionResult = m_DescriptorSets[setID].insert(std::make_pair(bindingID, binding));
            if(!insertionResult.second)
            {
                vkcv_log(LogLevel::WARNING,
                         "Attempting to overwrite already existing binding %u at set ID %u.",
                         bindingID,
                         setID);
            }
        }

        for (uint32_t i = 0; i < resources.separate_images.size(); i++) {
            auto& u = resources.separate_images[i];
            const spirv_cross::SPIRType &base_type = comp.get_type(u.base_type_id);
            const spirv_cross::SPIRType &type      = comp.get_type(u.type_id);

            uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            uint32_t descriptorCount = base_type.vecsize;
            bool variableCount = false;
            // query whether reflected resources are qualified as one-dimensional array
            if(type.array_size_literal[0])
            {
                descriptorCount = type.array[0];
                if(type.array[0] == 0)
                    variableCount = true;
            }

            DescriptorBinding binding {
				bindingID,
				DescriptorType::IMAGE_SAMPLED,
				descriptorCount,
				shaderStage,
				variableCount,
				variableCount // partialBinding == variableCount
			};

            auto insertionResult = m_DescriptorSets[setID].insert(std::make_pair(bindingID, binding));
            if(!insertionResult.second)
            {
                vkcv_log(LogLevel::WARNING,
                         "Attempting to overwrite already existing binding %u at set ID %u.",
                         bindingID,
                         setID);
            }
        }

        for (uint32_t i = 0; i < resources.storage_images.size(); i++) {
            auto& u = resources.storage_images[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);
            const spirv_cross::SPIRType &type      = comp.get_type(u.type_id);

            uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);

            uint32_t descriptorCount = base_type.vecsize;
            bool variableCount = false;
            // query whether reflected resources are qualified as one-dimensional array
            if(type.array_size_literal[0])
            {
                descriptorCount = type.array[0];
                if(type.array[0] == 0)
                    variableCount = true;
            }

            DescriptorBinding binding {
				bindingID,
				DescriptorType::IMAGE_STORAGE,
				descriptorCount,
				shaderStage,
				variableCount,
				variableCount // partialBinding == variableCount
			};

            auto insertionResult = m_DescriptorSets[setID].insert(std::make_pair(bindingID, binding));
            if(!insertionResult.second)
            {
                vkcv_log(LogLevel::WARNING,
                         "Attempting to overwrite already existing binding %u at set ID %u.",
                         bindingID,
                         setID);
            }
        }

        // Used to reflect acceleration structure bindings for RTX.
        for (uint32_t i = 0; i < resources.acceleration_structures.size(); i++) {
            auto& u = resources.acceleration_structures[i];
            const spirv_cross::SPIRType& base_type = comp.get_type(u.base_type_id);

            uint32_t setID = comp.get_decoration(u.id, spv::DecorationDescriptorSet);
            uint32_t bindingID = comp.get_decoration(u.id, spv::DecorationBinding);
            auto binding = DescriptorBinding {
                bindingID,
                DescriptorType::ACCELERATION_STRUCTURE_KHR,
                base_type.vecsize,
                shaderStage,
				false,
				false
			};

            auto insertionResult = m_DescriptorSets[setID].insert(std::make_pair(bindingID, binding));
            if (!insertionResult.second)
            {
                vkcv_log(LogLevel::WARNING,
                    "Attempting to overwrite already existing binding %u at set ID %u.",
                    bindingID,
                    setID);
            }
        }

        //reflect push constants
		for (const auto &pushConstantBuffer : resources.push_constant_buffers)
		{
			for (const auto &range : comp.get_active_buffer_ranges(pushConstantBuffer.id))
			{
				const size_t size = range.range + range.offset;
				m_pushConstantsSize = std::max(m_pushConstantsSize, size);
			}
		}
    }

	const std::unordered_map<uint32_t, DescriptorBindings>& ShaderProgram::getReflectedDescriptors() const
    {
        return m_DescriptorSets;
    }

	size_t ShaderProgram::getPushConstantsSize() const
	{
		return m_pushConstantsSize;
	}
}
