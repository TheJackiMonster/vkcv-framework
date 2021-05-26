#include "vkcv/DescriptorConfig.hpp"

#include <utility>

namespace vkcv {

    DescriptorBinding::DescriptorBinding(
        DescriptorType descriptorType,
        uint32_t descriptorCount,
        ShaderStage shaderStage
    ) noexcept :
        descriptorType{descriptorType},
        descriptorCount{descriptorCount},
        shaderStage{shaderStage}
    {};

    DescriptorSet::DescriptorSet(std::vector<DescriptorBinding> bindings) noexcept :
        bindings{std::move(bindings)}
    {};
}
