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
        /**
         * The material can be used for physically based rendering.
         */
		PBR_MATERIAL = 1,

        /**
         * The type is unknown.
         */
		UNKNOWN = 0
	};

    /**
     * Class to manage required handles for materials using
     * a wide range of textures with separate samplers and factors.
     */
	class Material {
	private:

        /**
         * A nested structure for textures used by a material.
         */
		struct Texture {
            /**
             * The image handle of a texture.
             */
			ImageHandle m_Image;

            /**
             * The sampler handle of a texture.
             */
			SamplerHandle m_Sampler;

            /**
             * The list of custom factors for a given texture.
             */
			std::vector<float> m_Factors;
		};

        /**
         * The type of a material.
         */
		MaterialType m_Type;

        /**
         * The descriptor set handle of a material.
         */
		DescriptorSetHandle m_DescriptorSet;

        /**
         * The descriptor set layout used by a material.
         */
		DescriptorSetLayoutHandle m_DescriptorSetLayout;

        /**
         * The list of textures used by a material.
         */
		std::vector<Texture> m_Textures;
		
	public:
        /**
         * Default constructor to create an invalid material instance.
         */
		Material();
		~Material() = default;
		
		Material(const Material& other) = default;
		Material(Material&& other) = default;
		
		Material& operator=(const Material& other) = default;
		Material& operator=(Material&& other) = default;

        /**
         * Returns the type of a material as MaterialType.
         * @return Type of material
         */
		[[nodiscard]]
		MaterialType getType() const;

        /**
         * Returns the descriptor set handle of the material.
         * @return Descriptor set handle
         */
		[[nodiscard]]
		const DescriptorSetHandle& getDescriptorSet() const;

        /**
         * Returns the descriptor set layout handle used by the material.
         * @return Descriptor set layout handle
         */
		[[nodiscard]]
		const DescriptorSetLayoutHandle& getDescriptorSetLayout() const;

        /**
         * Checks if the material is valid and returns the status
         * as boolean value.
         * @return true if the material is valid, otherwise false
         */
		explicit operator bool() const;

        /**
         * Checks if the material is invalid and returns the status
         * as boolean value.
         * @return true if the material is invalid, otherwise false
         */
		bool operator!() const;

        /**
         * Returns the descriptor bindings required by a given material
         * type to create the descriptor set layout.
         * @param[in] type Type of material
         * @return Descriptor bindings of a material type
         */
		static const DescriptorBindings& getDescriptorBindings(MaterialType type);

        /**
         * Creates a new valid material which supports physically based
         * rendering.
         * @param[in,out] core Reference to Core instance
         * @param[in] colorImg Base color image handle
         * @param[in] colorSmp Base color sampler handle
         * @param[in] normalImg Normal map image handle
         * @param[in] normalSmp Normal map sampler handle
         * @param[in] metRoughImg Metallic and roughness image handle
         * @param[in] metRoughSmp Metallic and roughness sampler handle
         * @param[in] occlusionImg Occlusion map image handle
         * @param[in] occlusionSmp Occlusion map sampler handle
         * @param[in] emissiveImg Emissive image handle
         * @param[in] emissiveSmp Emissive sampler handle
         * @param[in] baseColorFactor 4D vector of base color factors
         * @param[in] metallicFactor Metallic factor
         * @param[in] roughnessFactor Roughness factor
         * @param[in] normalScale Scale of normal map
         * @param[in] occlusionStrength Strength of occlusion
         * @param[in] emissiveFactor 3D vector of emmisive factors
         * @return New material instance
         */
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
