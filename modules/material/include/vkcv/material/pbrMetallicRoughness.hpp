#pragma once

#include <vector>

#include <vkcv/DescriptorConfig.hpp>
#include <vkcv/Core.hpp>


#include "Material.hpp"

namespace vkcv::material
{
    class PBRMaterial : Material
    {
    private:
        struct vec3 {
            float x, y, z;
        };
        struct vec4 {
            float x, y, z, a;
        };
        PBRMaterial(const ImageHandle& colorImg,
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
            vec3 emissiveFactor) noexcept;


    public:
        PBRMaterial() = delete;
       

        const ImageHandle   m_ColorTexture;
        const SamplerHandle m_ColorSampler;

        const ImageHandle   m_NormalTexture;
        const SamplerHandle m_NormalSampler;

        const ImageHandle   m_MetRoughTexture;
        const SamplerHandle m_MetRoughSampler;

        const ImageHandle m_OcclusionTexture;
        const SamplerHandle m_OcclusionSampler;

        const ImageHandle m_EmissiveTexture;
        const SamplerHandle m_EmissiveSampler;

        //
        vec4 m_BaseColorFactor;
        float m_MetallicFactor;
        float m_RoughnessFactor;
        float m_NormalScale;
        float m_OcclusionStrength;
        vec3 m_EmissiveFactor;

        /*
        * Returns the material's necessary descriptor bindings which serves as its descriptor layout
        * The binding is in the following order:
        * 0 - diffuse texture
        * 1 - diffuse sampler
        * 2 - normal texture
        * 3 - normal sampler
        * 4 - metallic roughness texture
        * 5 - metallic roughness sampler
        * 6 - occlusion texture
        * 7 - occlusion sampler
        * 8 - emissive texture
        * 9 - emissive sampler
        */
        static std::vector<DescriptorBinding> getDescriptorBindings() noexcept;

        static PBRMaterial create(
            vkcv::Core* core,
            ImageHandle          &colorImg,
            SamplerHandle        &colorSmp,
            ImageHandle          &normalImg,
            SamplerHandle        &normalSmp,
            ImageHandle          &metRoughImg,
            SamplerHandle        &metRoughSmp,
			ImageHandle			&occlusionImg,
			SamplerHandle		&occlusionSmp,
			ImageHandle			&emissiveImg,
			SamplerHandle		&emissiveSmp,
            vec4 baseColorFactor,
            float metallicFactor,
            float roughnessFactor,
            float normalScale,
            float occlusionStrength,
            vec3 emissiveFactor);

    };
}