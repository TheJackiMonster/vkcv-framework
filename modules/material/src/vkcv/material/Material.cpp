
#include "vkcv/material/Material.hpp"

#include <vkcv/Image.hpp>
#include <vkcv/Sampler.hpp>

namespace vkcv::material {
	
	Material::Material() {
		m_Type = MaterialType::UNKNOWN;
	}

	MaterialType Material::getType() const {
		return m_Type;
	}
	
	const DescriptorSetHandle & Material::getDescriptorSet() const {
		return m_DescriptorSet;
	}

	const DescriptorSetLayoutHandle & Material::getDescriptorSetLayout() const {
        return m_DescriptorSetLayout;
	}
	
	Material::operator bool() const {
		return (m_Type != MaterialType::UNKNOWN);
	}
	
	bool Material::operator!() const {
		return (m_Type == MaterialType::UNKNOWN);
	}
	
	void Material::recordMipChainGeneration(const vkcv::CommandStreamHandle& cmdStream,
											Downsampler &downsampler) {
		for (auto& texture : m_Textures) {
			downsampler.recordDownsampling(cmdStream, texture.m_Image);
		}
	}
	
	const DescriptorBindings& Material::getDescriptorBindings(MaterialType type)
	{
		static DescriptorBindings pbr_bindings = {};
		static DescriptorBindings default_bindings = {};
		
		switch (type) {
			case MaterialType::PBR_MATERIAL:
				if (pbr_bindings.empty())
				{
					pbr_bindings.insert(std::make_pair(0, DescriptorBinding {
						0, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(1, DescriptorBinding {
						1, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(2, DescriptorBinding {
						2, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(3, DescriptorBinding {
						3, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(4, DescriptorBinding {
						4, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(5, DescriptorBinding {
						5, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(6, DescriptorBinding {
						6, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(7, DescriptorBinding {
						7, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(8, DescriptorBinding {
						8, DescriptorType::IMAGE_SAMPLED, 1, ShaderStage::FRAGMENT, false
					}));
					pbr_bindings.insert(std::make_pair(9, DescriptorBinding {
						9, DescriptorType::SAMPLER, 1, ShaderStage::FRAGMENT, false
					}));
				}
				
				return pbr_bindings;
			default:
				return default_bindings;
		}
	}
	
	static void fillImage(Image& image, float data [4]) {
		std::vector<float> vec (image.getWidth() * image.getHeight() * image.getDepth() * 4);
		
		for (size_t i = 0; i < vec.size(); i++) {
			vec[i] = data[i % 4];
		}
		
		image.fill(vec.data());
	}
	
	Material Material::createPBR(Core &core,
								 const ImageHandle &colorImg, const SamplerHandle &colorSmp,
								 const ImageHandle &normalImg, const SamplerHandle &normalSmp,
								 const ImageHandle &metRoughImg, const SamplerHandle &metRoughSmp,
								 const ImageHandle &occlusionImg, const SamplerHandle &occlusionSmp,
								 const ImageHandle &emissiveImg, const SamplerHandle &emissiveSmp,
								 const float baseColorFactor [4],
								 float metallicFactor,
								 float roughnessFactor,
								 float normalScale,
								 float occlusionStrength,
								 const float emissiveFactor [3]) {
		ImageHandle images [5] = {
				colorImg, normalImg, metRoughImg, occlusionImg, emissiveImg
		};
		
		SamplerHandle samplers [5] = {
				colorSmp, normalSmp, metRoughSmp, occlusionSmp, emissiveSmp
		};
		
		if (!colorImg) {
			vkcv::Image defaultColor = image(core, vk::Format::eR8G8B8A8Srgb, 2, 2);
			float colorData [4] = { 228, 51, 255, 1 };
			fillImage(defaultColor, colorData);
			images[0] = defaultColor.getHandle();
		}
		
		if (!normalImg) {
			vkcv::Image defaultNormal = image(core, vk::Format::eR8G8B8A8Unorm, 2, 2);
			float normalData [4] = { 0, 0, 1, 0 };
			fillImage(defaultNormal, normalData);
			images[1] = defaultNormal.getHandle();
		}
		
		if (!metRoughImg) {
			vkcv::Image defaultRough = image(core, vk::Format::eR8G8B8A8Unorm, 2, 2);
			float roughData [4] = { 228, 51, 255, 1 };
			fillImage(defaultRough, roughData);
			images[2] = defaultRough.getHandle();
		}
		
		if (!occlusionImg) {
			vkcv::Image defaultOcclusion = image(core, vk::Format::eR8G8B8A8Unorm, 2, 2);
			float occlusionData [4] = { 228, 51, 255, 1 };
			fillImage(defaultOcclusion, occlusionData);
			images[3] = defaultOcclusion.getHandle();
		}
		
		if (!emissiveImg) {
			vkcv::Image defaultEmissive = image(core, vk::Format::eR8G8B8A8Srgb, 2, 2);
			float emissiveData [4] = { 0, 0, 0, 1 };
			fillImage(defaultEmissive, emissiveData);
			images[4] = defaultEmissive.getHandle();
		}
		
		if (!colorSmp) {
			samplers[0] = samplerLinear(core);
		}
		
		if (!normalSmp) {
			samplers[1] = samplerLinear(core);
		}
		
		if (!metRoughSmp) {
			samplers[2] = samplerLinear(core);
		}
		
		if (!occlusionSmp) {
			samplers[3] = samplerLinear(core);
		}
		
		if (!emissiveSmp) {
			samplers[4] = samplerLinear(core);
		}
		
		Material material;
		material.m_Type = MaterialType::PBR_MATERIAL;
		
		const auto& bindings = getDescriptorBindings(material.m_Type);
		material.m_DescriptorSetLayout = core.createDescriptorSetLayout(bindings);
		material.m_DescriptorSet = core.createDescriptorSet(material.m_DescriptorSetLayout);;
		
		material.m_Textures.reserve(bindings.size());
		material.m_Textures.push_back({ images[0], samplers[0], std::vector<float>(baseColorFactor, baseColorFactor+4) });
		material.m_Textures.push_back({ images[1], samplers[1], { normalScale } });
		material.m_Textures.push_back({ images[2], samplers[2], { metallicFactor, roughnessFactor } });
		material.m_Textures.push_back({ images[3], samplers[3], { occlusionStrength } });
		material.m_Textures.push_back({ images[4], samplers[4], std::vector<float>(emissiveFactor, emissiveFactor+3) });
		
		vkcv::DescriptorWrites setWrites;
		
		for (size_t i = 0; i < material.m_Textures.size(); i++) {
			setWrites.writeSampledImage(i * 2, material.m_Textures[i].m_Image);
			setWrites.writeSampler(i * 2 + 1, material.m_Textures[i].m_Sampler);
		}
		
		core.writeDescriptorSet(material.m_DescriptorSet, setWrites);
		return material;
	}

}
