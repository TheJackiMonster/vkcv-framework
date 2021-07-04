#pragma once

#include <vulkan/vulkan.hpp>

#include "vkcv/Handles.hpp"

namespace vkcv
{
    struct DescriptorSet
    {
        vk::DescriptorSet       vulkanHandle;
        vk::DescriptorSetLayout layout;
    };

    /*
    * All the types of descriptors (resources) that can be retrieved by the shaders
    */
    enum class DescriptorType
    {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,
        SAMPLER,
        IMAGE_SAMPLED,
		IMAGE_STORAGE
    };    
    
    /*
    * One binding for a descriptor set
    * @param[in] a unique binding ID
    * @param[in] a descriptor type
    * @param[in] the number of descriptors of this type (arrays of the same type possible)
    * @param[in] the shader stage where the descriptor is supposed to be retrieved
    */
    struct DescriptorBinding
    {
        DescriptorBinding(
            uint32_t             bindingID,
            DescriptorType       descriptorType,
            uint32_t             descriptorCount,
            vk::ShaderStageFlags shaderStages
        ) noexcept;
        
        uint32_t                bindingID;
        DescriptorType          descriptorType;
        uint32_t                descriptorCount;
        vk::ShaderStageFlags    shaderStages;
    };
}
