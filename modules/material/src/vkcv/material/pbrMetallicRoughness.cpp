#include "vkcv/material/pbrMetallicRoughness.hpp"


namespace vkcv::material
{
    PBRMaterial::PBRMaterial(
        const ImageHandle& colorImg,
        const SamplerHandle& colorSmp,
        const ImageHandle& normalImg,
        const SamplerHandle& normalSmp,
        const ImageHandle& metRoughImg,
        const SamplerHandle& metRoughSmp,
        const ImageHandle& occlusionImg,
        const SamplerHandle& occlusionSmp,
        const ImageHandle& emissiveImg,
        const SamplerHandle& emissiveSmp,
        const DescriptorSetHandle& setHandle,
        vec4 baseColorFactor,
        float metallicFactor,
        float roughnessFactor,
        float normalScale,
        float occlusionStrength,
        vec3 emissiveFactor) noexcept :
        m_ColorTexture(colorImg),
        m_ColorSampler(colorSmp),
        m_NormalTexture(normalImg),
        m_NormalSampler(normalSmp),
        m_MetRoughTexture(metRoughImg),
        m_MetRoughSampler(metRoughSmp),
        m_OcclusionTexture(occlusionImg),
        m_OcclusionSampler(occlusionSmp),
        m_EmissiveTexture(emissiveImg),
        m_EmissiveSampler(emissiveSmp),
        Material(setHandle),
        m_BaseColorFactor(baseColorFactor),
        m_MetallicFactor(metallicFactor),
        m_RoughnessFactor(roughnessFactor),
        m_NormalScale(normalScale),
        m_OcclusionStrength(occlusionStrength),
        m_EmissiveFactor(emissiveFactor)
    {
    }

    std::vector<DescriptorBinding> PBRMaterial::getDescriptorBindings() noexcept
    {
        return {{DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT},
                {DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
                {DescriptorType::SAMPLER      , 1, ShaderStage::FRAGMENT},
                {DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT},
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
        SamplerHandle& metRoughSmp,
        ImageHandle& occlusionImg,
        SamplerHandle& occlusionSmp,
        ImageHandle& emissiveImg,
        SamplerHandle& emissiveSmp,
        vec4 baseColorFactor,
        float metallicFactor,
        float roughnessFactor,
        float normalScale,
        float occlusionStrength,
        vec3 emissiveFactor)
    {
        //Test if Images and samplers valid, if not use default
        uint32_t width  = core->getImageWidth(colorImg); //use colorImg size as default
        uint32_t height = core->getImageHeight(colorImg);
        uint32_t n = width * height;
       

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
        if (!occlusionImg || (core->getImageWidth(occlusionImg) != width) || (core->getImageHeight(occlusionImg) != height)) {
            vkcv::Image defaultOcclusion = core->createImage(vk::Format::eR8G8B8A8Srgb, width, height);
            std::vector<vec4> occlusionData(n);
            std::fill(occlusionData.begin(), occlusionData.end(), vec4{ 228, 51 , 255, 1 });
            defaultOcclusion.fill(occlusionData.data());
            occlusionImg = defaultOcclusion.getHandle();
        }
        if (!emissiveImg || (core->getImageWidth(emissiveImg) != width) || (core->getImageHeight(emissiveImg) != height)) {
            vkcv::Image defaultEmissive = core->createImage(vk::Format::eR8G8B8A8Srgb, width, height);
            std::vector<vec4> emissiveData(n);
            std::fill(emissiveData.begin(), emissiveData.end(), vec4{ 228, 51 , 255, 1 });
            defaultEmissive.fill(emissiveData.data());
            emissiveImg = defaultEmissive.getHandle();
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
        if (!occlusionSmp) {
            occlusionSmp = core->createSampler(
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerFilterType::LINEAR,
                vkcv::SamplerMipmapMode::LINEAR,
                vkcv::SamplerAddressMode::REPEAT
            );
        }
        if (!emissiveSmp) {
            emissiveSmp = core->createSampler(
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
            vkcv::SampledImageDescriptorWrite(4, metRoughImg),
            vkcv::SampledImageDescriptorWrite(6, occlusionImg),
            vkcv::SampledImageDescriptorWrite(8, emissiveImg) };
        setWrites.samplerWrites = {
            vkcv::SamplerDescriptorWrite(1, colorSmp),
            vkcv::SamplerDescriptorWrite(3, normalSmp),
            vkcv::SamplerDescriptorWrite(5, metRoughSmp),
            vkcv::SamplerDescriptorWrite(7, occlusionSmp),
            vkcv::SamplerDescriptorWrite(9, emissiveSmp) };
        core->writeDescriptorSet(descriptorSetHandle, setWrites);

        return PBRMaterial(
            colorImg,
            colorSmp,
            normalImg,
            normalSmp,
            metRoughImg,
            metRoughSmp,
            occlusionImg,
            occlusionSmp,
            emissiveImg,
            emissiveSmp,
            descriptorSetHandle,
            baseColorFactor,
            metallicFactor,
            roughnessFactor,
            normalScale,
            occlusionStrength,
            emissiveFactor);
    }
}