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
        STORAGE_BUFFER,
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
            DescriptorType descriptorType,
            uint32_t descriptorCount,
            ShaderStage shaderStage
        ) noexcept;

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
        explicit DescriptorSet(std::vector<DescriptorBinding> bindings) noexcept;

        std::vector<DescriptorBinding> bindings;
    };
}
