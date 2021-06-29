
#include "vkcv/material/Material.hpp"

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
	
	bool Material::operator!() const {
		return (m_Type == MaterialType::UNKNOWN);
	}
	
	const std::vector<DescriptorBinding>& Material::getPBRDescriptorBindings() noexcept
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
	
	Material Material::createPBR(Core &core,
								 ImageHandle &colorImg, SamplerHandle &colorSmp,
								 ImageHandle &normalImg, SamplerHandle &normalSmp,
								 ImageHandle &metRoughImg, SamplerHandle &metRoughSmp,
								 ImageHandle &occlusionImg, SamplerHandle &occlusionSmp,
								 ImageHandle &emissiveImg, SamplerHandle &emissiveSmp,
								 float baseColorFactor [4],
								 float metallicFactor,
								 float roughnessFactor,
								 float normalScale,
								 float occlusionStrength,
								 float emissiveFactor [3]) {
		if (!colorImg) {
			vkcv::Image defaultColor = core.createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
			float colorData [4] = { 228, 51, 255, 1 };
			defaultColor.fill(&colorData);
			colorImg = defaultColor.getHandle();
		}
		
		if (!normalImg) {
			vkcv::Image defaultNormal = core.createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
			float normalData [4] = { 0, 0, 1, 0 };
			defaultNormal.fill(&normalData);
			normalImg = defaultNormal.getHandle();
		}
		
		if (!metRoughImg) {
			vkcv::Image defaultRough = core.createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
			float roughData [4] = { 228, 51, 255, 1 };
			defaultRough.fill(&roughData);
			metRoughImg = defaultRough.getHandle();
		}
		
		if (!occlusionImg) {
			vkcv::Image defaultOcclusion = core.createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
			float occlusionData [4] = { 228, 51, 255, 1 };
			defaultOcclusion.fill(&occlusionData);
			occlusionImg = defaultOcclusion.getHandle();
		}
		
		if (!emissiveImg) {
			vkcv::Image defaultEmissive = core.createImage(vk::Format::eR8G8B8A8Srgb, 1, 1);
			float emissiveData [4] = { 0, 0, 0, 1 };
			defaultEmissive.fill(&emissiveData);
			emissiveImg = defaultEmissive.getHandle();
		}
		
		if (!colorSmp) {
			colorSmp = core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT
			);
		}
		
		if (!normalSmp) {
			normalSmp = core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT
			);
		}
		
		if (!metRoughSmp) {
			metRoughSmp = core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT
			);
		}
		
		if (!occlusionSmp) {
			occlusionSmp = core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT
			);
		}
		
		if (!emissiveSmp) {
			emissiveSmp = core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT
			);
		}
		
		const auto& bindings = getPBRDescriptorBindings();
		vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(bindings);
		
		
		Material material;
		material.m_Type = MaterialType::PBR_MATERIAL;
		material.m_DescriptorSet = descriptorSet;
		
		material.m_Textures.reserve(bindings.size());
		material.m_Textures.push_back({ colorImg, colorSmp, std::vector<float>(baseColorFactor, baseColorFactor+4) });
		material.m_Textures.push_back({ normalImg, normalSmp, std::vector<float>(&normalScale, &normalScale+1) });
		material.m_Textures.push_back({ metRoughImg, metRoughSmp, std::vector<float>(&metallicFactor, &metallicFactor+1) });
		material.m_Textures.push_back({ occlusionImg, occlusionSmp, std::vector<float>(&occlusionStrength, &occlusionStrength+1) });
		material.m_Textures.push_back({ emissiveImg, emissiveSmp, std::vector<float>(emissiveFactor, emissiveFactor+3) });
		
		vkcv::DescriptorWrites setWrites;
		
		for (size_t i = 0; i < material.m_Textures.size(); i++) {
			setWrites.sampledImageWrites.emplace_back(i * 2, material.m_Textures[i].m_Image);
			setWrites.samplerWrites.emplace_back(i * 2 + 1, material.m_Textures[i].m_Sampler);
		}
		
		core.writeDescriptorSet(descriptorSet, setWrites);
		return material;
	}

}
