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
    void pbrMaterial::create(vkcv::Core core)
    {   
        //Test if Images and samplers valid
        //create default
        vkcv::SamplerHandle defaultSampler = core.createSampler(
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerMipmapMode::LINEAR,
            vkcv::SamplerAddressMode::REPEAT
        );         
        //create descriptorset
        vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(getDescriptorBindings());       
        //writes
        vkcv::DescriptorWrites setWrites;
        setWrites.sampledImageWrites = {
            vkcv::SampledImageDescriptorWrite(0, m_ColorTexture),
            vkcv::SampledImageDescriptorWrite(2, m_NormalTexture),
            vkcv::SampledImageDescriptorWrite(4, m_MetRoughTexture) };
        setWrites.samplerWrites = {
            vkcv::SamplerDescriptorWrite(1, m_ColorSampler),
            vkcv::SamplerDescriptorWrite(3, m_NormalSampler), 
            vkcv::SamplerDescriptorWrite(5, m_MetRoughSampler) };
        core.writeResourceDescription(descriptorSet, 0, setWrites);
    }
}