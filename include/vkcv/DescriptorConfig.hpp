#pragma once

#include <unordered_map>

#include "vkcv/Handles.hpp"
#include "vkcv/ShaderStage.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv
{
    /*
    * All the types of descriptors (resources) that can be retrieved by the shaders
    */
    enum class DescriptorType
    {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,
        SAMPLER,
        IMAGE_SAMPLED,
		IMAGE_STORAGE,
        UNIFORM_BUFFER_DYNAMIC,
        STORAGE_BUFFER_DYNAMIC
    };    

    /**
    * Converts the descriptor types from VulkanCV (vkcv) to native Vulkan (vk).
    * @param[in] vkcv DescriptorType
    * @return vk DescriptorType
    */
    constexpr vk::DescriptorType getVkDescriptorType(DescriptorType type) noexcept {
        switch (type)
        {
            case DescriptorType::UNIFORM_BUFFER:
                return vk::DescriptorType::eUniformBuffer;
            case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
                return vk::DescriptorType::eUniformBufferDynamic;
            case DescriptorType::STORAGE_BUFFER:
                return vk::DescriptorType::eStorageBuffer;
            case DescriptorType::STORAGE_BUFFER_DYNAMIC:
                return vk::DescriptorType::eStorageBufferDynamic;
            case DescriptorType::SAMPLER:
                return vk::DescriptorType::eSampler;
            case DescriptorType::IMAGE_SAMPLED:
                return vk::DescriptorType::eSampledImage;
            case DescriptorType::IMAGE_STORAGE:
                return vk::DescriptorType::eStorageImage;
            default:
                return vk::DescriptorType::eMutableVALVE;
        }
    }

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
            uint32_t bindingID,
            DescriptorType descriptorType,
            uint32_t descriptorCount,
            ShaderStages shaderStages
        ) noexcept;
        
        uint32_t        bindingID;
        DescriptorType  descriptorType;
        uint32_t        descriptorCount;
        ShaderStages    shaderStages;

        bool DescriptorBinding::operator ==(const DescriptorBinding &other) const;
    };
    
    typedef std::unordered_map<uint32_t, DescriptorBinding> DescriptorBindings;

    struct DescriptorSetLayout
    {
        vk::DescriptorSetLayout vulkanHandle;
        DescriptorBindings descriptorBindings;
    };

    struct DescriptorSet
    {
        vk::DescriptorSet           vulkanHandle;
        DescriptorSetLayoutHandle   setLayoutHandle;
        size_t                      poolIndex;
    };
}
