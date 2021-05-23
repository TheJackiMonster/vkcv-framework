#pragma once
#include <vkcv/ShaderProgram.hpp>

namespace vkcv
{
    enum class DescriptorType
    {
        // TODO:
        //  uniform buffers, samplers, images should be supported for now!
        UNIFORM_BUFFER,
        SAMPLER,
        IMAGE
    };

    struct DescriptorBinding
    {
        // TODO:
        //  should contain something like the binding ID,
        //  and the descriptor/resource type

        uint32_t bindingID;
        DescriptorType descriptorType;
        uint32_t descriptorCount;
        ShaderStage shaderStage;
    };

    struct DescriptorSet
    {
        // TODO:
        //  should contain a collection of DescriptorBindings and the number of instances to be created from the set
        std::vector<DescriptorBinding> bindings;
        uint32_t setCount;
    };
}
