#include "vkcv/material/pbrMetallicRoughness.hpp"

namespace vkcv::material
{
    pbrMaterial::pbrMaterial(const ImageHandle          &colorImg,
                             const SamplerHandle        &colorSmp,
                             const ImageHandle          &normalImg,
                             const SamplerHandle        &normalSmp,
                             const ImageHandle          &metRoughImg,
                             const SamplerHandle        &metRoughSmp,
                             const DescriptorSetHandle  &setHandle) noexcept :
    m_ColorTexture(colorImg),
    m_ColorSampler(colorSmp),
    m_NormalTexture(normalImg),
    m_NormalSampler(normalSmp),
    m_MetRoughTexture(metRoughImg),
    m_MetRoughSampler(metRoughSmp),
    m_DescriptorSetHandle(setHandle)
    {}

    std::vector<DescriptorBinding> pbrMaterial::getDescriptorBindings() noexcept
    {
        return {{DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT},
                {DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT},
                {DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT}};
    }
}