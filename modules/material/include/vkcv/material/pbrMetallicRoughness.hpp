#pragma once

#include <vector>

#include <vkcv/Handles.hpp>
#include <vkcv/DescriptorConfig.hpp>
#include <vkcv/Core.hpp>


#include "Material.hpp"

namespace vkcv::material
{
    class Core;
    class PBRMaterial : Material
    {
    public:
        PBRMaterial() = delete;
        PBRMaterial(const ImageHandle           &colorImg,
                    const SamplerHandle         &colorSmp,
                    const ImageHandle           &normalImg,
                    const SamplerHandle         &normalSmp,
                    const ImageHandle           &metRoughImg,
                    const SamplerHandle         &metRoughSmp,
                    const DescriptorSetHandle   &setHandle) noexcept;

        const ImageHandle   m_ColorTexture;
        const SamplerHandle m_ColorSampler;

        const ImageHandle   m_NormalTexture;
        const SamplerHandle m_NormalSampler;

        const ImageHandle   m_MetRoughTexture;
        const SamplerHandle m_MetRoughSampler;

        // ImageHandle m_OcclusionTexture;
        // SamplerHandle m_EmissiveTexture;

        const DescriptorSetHandle m_DescriptorSetHandle;

        /*
        * Returns the material's necessary descriptor bindings which serves as its descriptor layout
        * The binding is in the following order:
        * 0 - diffuse texture
        * 1 - diffuse sampler
        * 2 - normal texture
        * 3 - normal sampler
        * 4 - metallic roughness texture
        * 5 - metallic roughness sampler
        */
        static std::vector<DescriptorBinding> getDescriptorBindings() noexcept;

        void create(vkcv::Core core);

    };
}