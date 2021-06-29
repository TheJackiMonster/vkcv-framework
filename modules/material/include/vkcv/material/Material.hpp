#pragma once

#include <vector>

#include <vkcv/Core.hpp>
#include <vkcv/Handles.hpp>

namespace vkcv::material {
	
	enum class MaterialType {
		PBR_MATERIAL = 1,
		
		UNKNOWN = 0
	};
	
	class Material {
	private:
		struct Texture {
			ImageHandle m_Image;
			SamplerHandle m_Sampler;
			std::vector<float> m_Factors;
		};
		
		MaterialType m_Type;
		DescriptorSetHandle m_DescriptorSet;
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
		
		bool operator!() const;
		
		static const std::vector<DescriptorBinding>& getPBRDescriptorBindings() noexcept;
		
		static Material createPBR(Core &core,
								  ImageHandle &colorImg,
								  SamplerHandle &colorSmp,
								  ImageHandle &normalImg,
								  SamplerHandle &normalSmp,
								  ImageHandle &metRoughImg,
								  SamplerHandle &metRoughSmp,
								  ImageHandle &occlusionImg,
								  SamplerHandle &occlusionSmp,
								  ImageHandle &emissiveImg,
								  SamplerHandle &emissiveSmp,
								  float baseColorFactor [4],
								  float metallicFactor,
								  float roughnessFactor,
								  float normalScale,
								  float occlusionStrength,
								  float emissiveFactor [3]);
	
	};
	
}
