#pragma once
/**
 * @authors Artur Wasmut, Tobias Frisch, Simeon Hermann, Alexander Gauggel, Vanessa Karolek
 * @file vkcv/DescriptorConfig.hpp
 * @brief Structures to handle descriptor types and bindings.
 */

#include <unordered_map>

#include "Handles.hpp"
#include "ShaderStage.hpp"
#include "Logger.hpp"

namespace vkcv
{

    enum class DescriptorType {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,
        SAMPLER,
        IMAGE_SAMPLED,
		IMAGE_STORAGE,
        UNIFORM_BUFFER_DYNAMIC,
        STORAGE_BUFFER_DYNAMIC,
        ACCELERATION_STRUCTURE_KHR
    };

    /**
     * @brief Converts the descriptor type from the frameworks enumeration
     * to the Vulkan type specifier.
     *
     * @param[in] type Descriptor type
     * @return Vulkan descriptor type
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
            case DescriptorType::ACCELERATION_STRUCTURE_KHR:
                return vk::DescriptorType::eAccelerationStructureKHR;
            default:
                return vk::DescriptorType::eMutableVALVE;
        }
    }
	
    struct DescriptorBinding
    {
        uint32_t        bindingID;
        DescriptorType  descriptorType;
        uint32_t        descriptorCount;
        ShaderStages    shaderStages;
        bool            variableCount;

        bool operator ==(const DescriptorBinding &other) const;
    };
    
    typedef std::unordered_map<uint32_t, DescriptorBinding> DescriptorBindings;

    struct DescriptorSetLayout
    {
        vk::DescriptorSetLayout vulkanHandle;
        DescriptorBindings      descriptorBindings;
        size_t                  layoutUsageCount;
    };

    struct DescriptorSet
    {
        vk::DescriptorSet           vulkanHandle;
        DescriptorSetLayoutHandle   setLayoutHandle;
        size_t                      poolIndex;
    };
	
}
