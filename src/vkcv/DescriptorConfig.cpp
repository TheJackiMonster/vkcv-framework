#include "vkcv/DescriptorConfig.hpp"

namespace vkcv {

    DescriptorBinding::DescriptorBinding(
        uint32_t p_bindingID,
        DescriptorType p_descriptorType,
        uint32_t p_descriptorCount,
        ShaderStage p_shaderStage
    ) noexcept :
        bindingID{ p_bindingID },
        descriptorType{ p_descriptorType },
        descriptorCount{ p_descriptorCount },
        shaderStage{ p_shaderStage }
    {};

    DescriptorSet::DescriptorSet(
        std::vector<DescriptorBinding> p_bindings,
        uint32_t p_setCount
    ) noexcept :
        bindings{ p_bindings },
        setCount{ p_setCount }
    {};

}