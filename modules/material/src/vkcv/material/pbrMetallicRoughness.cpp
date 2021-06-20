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
        uint32_t width  = core->getImageWidth(colorImg); //use colorImg size as default
        uint32_t height = core->getImageHeight(colorImg);
        uint32_t n = width * height;
        struct vec3 {
            float x, y, z;
        };
        struct vec4 {
            float x, y, z, a;
        }; 

        if (!colorImg) {
            width = core->getImageWidth(metRoughImg); // if colorImg has no size
            height = core->getImageHeight(metRoughImg);
            n = width * height;
            vkcv::Image defaultColor = core->createImage(vk::Format::eR8G8B8A8Srgb, width, height);
            std::vector<vec4> colorData(n);
            std::fill(colorData.begin(), colorData.end(), vec4{ 228, 51 , 255, 1 });
            defaultColor.fill(colorData.data());
            colorImg = defaultColor.getHandle();
        }
        if (!normalImg || (core->getImageWidth(normalImg)!=width)|| (core->getImageHeight(normalImg) != height)) {
            vkcv::Image defaultNormal = core->createImage(vk::Format::eR8G8B8A8Srgb, width, height);
            std::vector<vec4> normalData(n);
            std::fill(normalData.begin(), normalData.end(), vec4{ 228, 51 , 255, 1 });
            defaultNormal.fill(normalData.data());
            normalImg = defaultNormal.getHandle();
        }
        if (!metRoughImg || (core->getImageWidth(metRoughImg) != width) || (core->getImageHeight(metRoughImg) != height)) {
            vkcv::Image defaultRough = core->createImage(vk::Format::eR8G8B8A8Srgb, width, height);
            std::vector<vec4> roughData(n);
            std::fill(roughData.begin(), roughData.end(), vec4{ 228, 51 , 255, 1 });
            defaultRough.fill(roughData.data());
            metRoughImg = defaultRough.getHandle();
        }
        if (!colorSmp) {            
            colorSmp = core->createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            );            
        }
        if (!normalSmp) {            
            normalSmp = core->createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            );            
        }
        if (!metRoughSmp) {
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
        core->writeDescriptorSet(descriptorSetHandle, setWrites);

        return PBRMaterial(colorImg, colorSmp, normalImg, normalSmp, metRoughImg, metRoughSmp, descriptorSetHandle);        
    }
}