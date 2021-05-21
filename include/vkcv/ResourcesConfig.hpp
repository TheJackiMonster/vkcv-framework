#pragma once

namespace vkcv
{
    enum class ResourceType
    {
        // TODO:
        //  uniform buffers, samplers, images should be supported for now!
    };

    struct BindingDescription
    {
        // TODO:
        //  should contain something like the binding ID,
        //  and the descriptor/resource type
    };

    struct SetDescription
    {
        // TODO:
        //  should contain a collection of BindingDescriptions
    };
}
