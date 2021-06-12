#include "vkcv/material/pbrMetallicRoughness.hpp"


namespace vkcv::material
{
    PBRMaterial::PBRMaterial(const ImageHandle          &colorImg,
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

    std::vector<DescriptorBinding> PBRMaterial::getDescriptorBindings() noexcept
    {
        return {{DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT},
                {DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT},
                {DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT}};
    }
    void PBRMaterial::create(vkcv::Core core)
    {   
        //Test if Images and samplers valid, if not use default
        if (m_ColorTexture) {
            //TODO
        }
        if (m_NormalTexture) {
            //TODO
        }
        if (m_MetRoughTexture) {
            //TODO
        }
        if (m_ColorSampler) {
            /*
            m_ColorSampler = core.createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            );//only non const member
            */
        }
        if (m_NormalSampler) {
            /*
            m_NormalSampler = core.createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            ); //only non const member
            */
        }
        if (m_MetRoughSampler) {
            /*
            m_MetRoughSampler = core.createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            ); //only non const member
            */
        }
          
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