
#include "vkcv/scene/Mesh.hpp"
#include "vkcv/scene/Scene.hpp"

namespace vkcv::scene {
	
	MeshPart::MeshPart(Scene* scene) :
	m_scene(scene) {}
	
	void loadImage(Core& core, const asset::Scene& asset_scene,
				   const asset::Texture& asset_texture,
				   const vk::Format& format,
				   ImageHandle& image, SamplerHandle& sampler) {
		asset::Sampler* asset_sampler = nullptr;
		
		if ((asset_texture.sampler >= 0) && (asset_texture.sampler < asset_scene.samplers.size())) {
			//asset_sampler = &(asset_scene.samplers[asset_texture.sampler]); // TODO
		}
		
		Image img = core.createImage(format, asset_texture.w, asset_texture.h, 1, true);
		img.fill(asset_texture.data.data());
		image = img.getHandle();
		
		if (asset_sampler) {
			//sampler = core.createSampler(asset_sampler) // TODO
		}
	}
	
	material::Material loadMaterial(Core& core, const asset::Scene& scene,
					  				const asset::Material& material) {
		ImageHandle diffuseImg;
		SamplerHandle diffuseSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		ImageHandle normalImg;
		SamplerHandle normalSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		ImageHandle metalRoughImg;
		SamplerHandle metalRoughSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		ImageHandle occlusionImg;
		SamplerHandle occlusionSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		ImageHandle emissionImg;
		SamplerHandle emissionSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		const float colorFactors [4] = {
				material.baseColorFactor.r,
				material.baseColorFactor.g,
				material.baseColorFactor.b,
				material.baseColorFactor.a
		};
		
		const float emissionFactors[4] = {
				material.emissiveFactor.r,
				material.emissiveFactor.g,
				material.emissiveFactor.b
		};
		
		return material::Material::createPBR(
				core,
				diffuseImg, diffuseSmp,
				normalImg, normalSmp,
				metalRoughImg, metalRoughSmp,
				occlusionImg, occlusionSmp,
				emissionImg, emissionSmp,
				colorFactors,
				material.normalScale,
				material.metallicFactor,
				material.roughnessFactor,
				material.occlusionStrength,
				emissionFactors
		);
	}
	
	void MeshPart::load(const asset::Scene& scene,
						const asset::VertexGroup &vertexGroup) {
		Core& core = *(m_scene->m_core);
		
		auto vertexBuffer = core.createBuffer<uint8_t>(
				BufferType::VERTEX, vertexGroup.vertexBuffer.data.size()
		);
		
		vertexBuffer.fill(vertexGroup.vertexBuffer.data);
		m_vertices = vertexBuffer.getHandle();
		
		auto indexBuffer = core.createBuffer<uint8_t>(
				BufferType::INDEX, vertexGroup.indexBuffer.data.size()
		);
		
		indexBuffer.fill(vertexGroup.indexBuffer.data);
		m_indices = indexBuffer.getHandle();
		
		m_bounds.setMin(glm::vec3(
				vertexGroup.min.x,
				vertexGroup.min.y,
				vertexGroup.min.z
		));
		
		m_bounds.setMax(glm::vec3(
				vertexGroup.max.x,
				vertexGroup.max.y,
				vertexGroup.max.z
		));
		
		if ((vertexGroup.materialIndex >= 0) &&
			(vertexGroup.materialIndex < scene.materials.size())) {
			m_materialIndex = vertexGroup.materialIndex;
			
			auto& material = m_scene->m_materials[m_materialIndex];
			
			if (0 == material.m_usages++) {
				material.m_data = loadMaterial(core, scene, scene.materials[vertexGroup.materialIndex]);
			}
		} else {
			m_materialIndex = std::numeric_limits<size_t>::max();
		}
	}
	
	MeshPart::~MeshPart() {
		if (m_materialIndex < std::numeric_limits<size_t>::max()) {
			auto& material = m_scene->m_materials[m_materialIndex];
			
			if (material.m_usages > 0) {
				material.m_usages--;
			}
		}
	}
	
	const material::Material & MeshPart::getMaterial() const {
		if (m_materialIndex < std::numeric_limits<size_t>::max()) {
			return m_scene->m_materials[m_materialIndex].m_data;
		} else {
			static material::Material noMaterial;
			return noMaterial;
		}
	}
	
	Mesh::Mesh(Scene* scene) :
	m_scene(scene) {}
	
	void Mesh::load(const asset::Scene &scene, const asset::Mesh &mesh) {
		for (const auto& vertexGroupIndex : mesh.vertexGroups) {
			if ((vertexGroupIndex < 0) || (vertexGroupIndex >= scene.vertexGroups.size())) {
				continue;
			}
			
			MeshPart part (m_scene);
			part.load(scene, scene.vertexGroups[vertexGroupIndex]);
			m_parts.push_back(part);
		}
	}

}
