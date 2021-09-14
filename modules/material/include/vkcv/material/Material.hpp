#pragma once

#include <vector>

#include <vkcv/Core.hpp>
#include <vkcv/Handles.hpp>

namespace vkcv::material {

    /**
     * @defgroup vkcv_material Material Module
     * A module to manage standardized materials for rendering.
     * @{
     */

    /**
     * Enum to handle standardized material types.
     */
	enum class MaterialType {
		PBR_MATERIAL = 1,
		
		UNKNOWN = 0
	};

    /**
     * Class to manage required handles for materials using
     * a wide range of textures with separate samplers and factors.
     */
	class Material {
	private:
		struct Texture {
			ImageHandle m_Image;
			SamplerHandle m_Sampler;
			std::vector<float> m_Factors;
		};
		
		MaterialType m_Type;
		DescriptorSetHandle m_DescriptorSet;
		DescriptorSetLayoutHandle m_DescriptorSetLayout;
		std::vector<Texture> m_Textures;
		
	public:
		Material();
		~Material() = default;
		
		Material(const Material& other) = default;
		Material(Material&& other) = default;
		
		Material& operator=(const Material& other) = default;
		Material& operator=(Material&& other) = default;
		
		[[nodiscard]]
		MaterialType getType() const;
		
		[[nodiscard]]
		const DescriptorSetHandle& getDescriptorSet() const;

		[[nodiscard]]
		const DescriptorSetLayoutHandle& getDescriptorSetLayout() const;
		
		explicit operator bool() const;
		
		bool operator!() const;
		
		static const std::unordered_map<uint32_t ,DescriptorBinding>& getDescriptorBindings(MaterialType type);
		
		static Material createPBR(Core &core,
								  const ImageHandle &colorImg,
								  const SamplerHandle &colorSmp,
								  const ImageHandle &normalImg,
								  const SamplerHandle &normalSmp,
								  const ImageHandle &metRoughImg,
								  const SamplerHandle &metRoughSmp,
								  const ImageHandle &occlusionImg,
								  const SamplerHandle &occlusionSmp,
								  const ImageHandle &emissiveImg,
								  const SamplerHandle &emissiveSmp,
								  const float baseColorFactor [4],
								  float metallicFactor,
								  float roughnessFactor,
								  float normalScale,
								  float occlusionStrength,
								  const float emissiveFactor [3]);
	
	};

    /** @} */
	
}
