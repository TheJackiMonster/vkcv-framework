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
		static std::vector<DescriptorBinding> bindings;
		
		if (bindings.empty()) {
			bindings.emplace_back(0, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(1, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(2, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(3, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(4, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(5, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(6, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(7, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(8, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT);
			bindings.emplace_back(9, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT);
		}
    	
        return bindings;
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
         if (!colorImg) {
            vkcv::Image defaultColor = core->createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
            vec4 colorData{ 228, 51, 255,1 };
            defaultColor.fill(&colorData);
            colorImg = defaultColor.getHandle();
        }
        if (!normalImg) {
            vkcv::Image defaultNormal = core->createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
            vec4 normalData{ 0, 0, 1,0 };
            defaultNormal.fill(&normalData);
            normalImg = defaultNormal.getHandle();
        }
        if (!metRoughImg) {
            vkcv::Image defaultRough = core->createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
            vec4 roughData{ 228, 51, 255,1 };
            defaultRough.fill(&roughData);
            metRoughImg = defaultRough.getHandle();
        }
        if (!occlusionImg) {
            vkcv::Image defaultOcclusion = core->createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
            vec4 occlusionData{ 228, 51, 255,1 };
            defaultOcclusion.fill(&occlusionData);
            occlusionImg = defaultOcclusion.getHandle();
        }
        if (!emissiveImg) {
            vkcv::Image defaultEmissive = core->createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
            vec4 emissiveData{ 0, 0, 0,1 };
            defaultEmissive.fill(&emissiveData);
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