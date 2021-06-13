#include "vkcv/material/pbrMetallicRoughness.hpp"


namespace vkcv::material
{
    PBRMaterial::PBRMaterial(
        const ImageHandle          &colorImg,
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
        Material(setHandle)        
    {
    }

    std::vector<DescriptorBinding> PBRMaterial::getDescriptorBindings() noexcept
    {
        return {{DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT},
                {DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT},
                {DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT}};
    }
    PBRMaterial PBRMaterial::create(
        vkcv::Core* core,
        ImageHandle& colorImg,
        SamplerHandle& colorSmp,
        ImageHandle& normalImg,
        SamplerHandle& normalSmp,
        ImageHandle& metRoughImg,
        SamplerHandle& metRoughSmp)
    {
        //Test if Images and samplers valid, if not use default
        if (colorImg) {
            //TODO
        }
        if (normalImg) {
            //TODO
        }
        if (metRoughImg) {
            //TODO
        }
        if (colorSmp) {            
            colorSmp = core->createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            );            
        }
        if (normalSmp) {            
            normalSmp = core->createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            );            
        }
        if (metRoughSmp) {
            metRoughSmp = core->createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            );            
        }


        //create descriptorset
        vkcv::DescriptorSetHandle descriptorSetHandle = core->createDescriptorSet(getDescriptorBindings());
        //writes
        vkcv::DescriptorWrites setWrites;
        setWrites.sampledImageWrites = {
            vkcv::SampledImageDescriptorWrite(0, colorImg),
            vkcv::SampledImageDescriptorWrite(2, normalImg),
            vkcv::SampledImageDescriptorWrite(4, metRoughImg) };
        setWrites.samplerWrites = {
            vkcv::SamplerDescriptorWrite(1, colorSmp),
            vkcv::SamplerDescriptorWrite(3, normalSmp),
            vkcv::SamplerDescriptorWrite(5, metRoughSmp) };
        core->writeResourceDescription(descriptorSetHandle, 0, setWrites);

        return PBRMaterial(colorImg, colorSmp, normalImg, normalSmp, metRoughImg, metRoughSmp, descriptorSetHandle);        
    }
}