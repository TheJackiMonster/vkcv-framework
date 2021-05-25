#pragma once
#include <vkcv/ShaderProgram.hpp>

namespace vkcv
{
    /*
    * All the types of descriptors (resources) that can be retrieved by the shaders
    */
    enum class DescriptorType
    {
        UNIFORM_BUFFER,
        SAMPLER,
        IMAGE
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
        DescriptorBinding() = delete;

        DescriptorBinding(
            uint32_t p_bindingID,
            DescriptorType p_descriptorType,
            uint32_t p_descriptorCount,
            ShaderStage p_shaderStage
        ) noexcept;

        uint32_t bindingID;
        DescriptorType descriptorType;
        uint32_t descriptorCount;
        ShaderStage shaderStage;
    };

    /*
    * One descriptor set struct that contains all the necessary information for the actual creation.
    * @param[in] a number of bindings that were created beforehand
    * @param[in] the number of (identical) sets that should be created from the attached bindings
    */
    struct DescriptorSet
    {
        DescriptorSet() = delete;

        DescriptorSet(
            std::vector<DescriptorBinding> p_bindings,
            uint32_t p_setCount) noexcept;

        std::vector<DescriptorBinding> bindings;
        uint32_t setCount;
    };
}
