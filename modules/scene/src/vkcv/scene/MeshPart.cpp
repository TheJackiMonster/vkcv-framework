
#include "vkcv/scene/MeshPart.hpp"
#include "vkcv/scene/Scene.hpp"

namespace vkcv::scene {
	
	MeshPart::MeshPart(Scene* scene) :
			m_scene(scene),
			m_vertices(),
			m_vertexBindings(),
			m_indices(),
			m_indexCount(0),
			m_bounds(),
			m_materialIndex(std::numeric_limits<size_t>::max()) {}
	
	static void loadImage(Core& core, const asset::Scene& asset_scene,
				   const asset::Texture& asset_texture,
				   const vk::Format& format,
				   ImageHandle& image, SamplerHandle& sampler) {
		asset::Sampler* asset_sampler = nullptr;
		
		if ((asset_texture.sampler >= 0) && (asset_texture.sampler < asset_scene.samplers.size())) {
			//asset_sampler = &(asset_scene.samplers[asset_texture.sampler]); // TODO
		}
		
		Image img = core.createImage(format, asset_texture.w, asset_texture.h);
		img.fill(asset_texture.data.data());
		image = img.getHandle();
		
		if (asset_sampler) {
			//sampler = core.createSampler(asset_sampler) // TODO
		} else {
			sampler = core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT
			);
		}
	}
	
	static material::Material loadMaterial(Core& core, const asset::Scene& scene,
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
						const asset::VertexGroup &vertexGroup,
						std::vector<DrawcallInfo>& drawcalls) {
		Core& core = *(m_scene->m_core);
		
		auto vertexBuffer = core.createBuffer<uint8_t>(
				BufferType::VERTEX, vertexGroup.vertexBuffer.data.size()
		);
		
		vertexBuffer.fill(vertexGroup.vertexBuffer.data);
		m_vertices = vertexBuffer.getHandle();
		
		auto attributes = vertexGroup.vertexBuffer.attributes;
		
		std::sort(attributes.begin(), attributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
			return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
		});
		
		for (const auto& attribute : attributes) {
			m_vertexBindings.emplace_back(attribute.offset, vertexBuffer.getVulkanHandle());
		}
		
		auto indexBuffer = core.createBuffer<uint8_t>(
				BufferType::INDEX, vertexGroup.indexBuffer.data.size()
		);
		
		indexBuffer.fill(vertexGroup.indexBuffer.data);
		m_indices = indexBuffer.getHandle();
		m_indexCount = vertexGroup.numIndices;
		
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
		
		if (*this) {
			const auto& material = getMaterial();
			const auto& descriptorSet = core.getDescriptorSet(material.getDescriptorSet());
			
			drawcalls.push_back(DrawcallInfo(
					vkcv::Mesh(m_vertexBindings, indexBuffer.getVulkanHandle(), m_indexCount),
					{ DescriptorSetUsage(0, descriptorSet.vulkanHandle) }
			));
		}
	}
	
	MeshPart::~MeshPart() {
		if ((m_scene->m_core) && (m_materialIndex < m_scene->m_materials.size())) {
			auto& material = m_scene->m_materials[m_materialIndex];
			
			if (material.m_usages > 0) {
				material.m_usages--;
			}
		}
	}
	
	MeshPart::MeshPart(const MeshPart &other) :
	m_scene(other.m_scene),
	m_vertices(other.m_vertices),
	m_vertexBindings(other.m_vertexBindings),
	m_indices(other.m_indices),
	m_indexCount(other.m_indexCount),
	m_bounds(other.m_bounds),
	m_materialIndex(other.m_materialIndex) {
		if (m_materialIndex < std::numeric_limits<size_t>::max()) {
			auto& material = m_scene->m_materials[m_materialIndex];
			
			material.m_usages++;
		}
	}
	
	MeshPart::MeshPart(MeshPart &&other) noexcept :
	m_scene(other.m_scene),
	m_vertices(other.m_vertices),
	m_vertexBindings(other.m_vertexBindings),
	m_indices(other.m_indices),
	m_indexCount(other.m_indexCount),
	m_bounds(other.m_bounds),
	m_materialIndex(other.m_materialIndex) {
		other.m_materialIndex = std::numeric_limits<size_t>::max();
	}
	
	MeshPart &MeshPart::operator=(const MeshPart &other) {
		if (&other == this) {
			return *this;
		}
		
		m_scene = other.m_scene;
		m_vertices = other.m_vertices;
		m_vertexBindings = other.m_vertexBindings;
		m_indices = other.m_indices;
		m_indexCount = other.m_indexCount;
		m_bounds = other.m_bounds;
		m_materialIndex = other.m_materialIndex;
		
		return *this;
	}
	
	MeshPart &MeshPart::operator=(MeshPart &&other) noexcept {
		m_scene = other.m_scene;
		m_vertices = other.m_vertices;
		m_vertexBindings = other.m_vertexBindings;
		m_indices = other.m_indices;
		m_indexCount = other.m_indexCount;
		m_bounds = other.m_bounds;
		m_materialIndex = other.m_materialIndex;
		
		other.m_materialIndex = std::numeric_limits<size_t>::max();
		
		return *this;
	}
	
	const material::Material & MeshPart::getMaterial() const {
		static material::Material noMaterial;
		
		if (m_materialIndex < m_scene->m_materials.size()) {
			return m_scene->m_materials[m_materialIndex].m_data;
		} else {
			return noMaterial;
		}
	}
	
	MeshPart::operator bool() const {
		return (
				(m_materialIndex < m_scene->m_materials.size()) &&
				(m_vertices) &&
				(m_indices)
		);
	}
	
	bool MeshPart::operator!() const {
		return (
				(m_materialIndex >= m_scene->m_materials.size()) ||
				(!m_vertices) ||
				(!m_indices)
		);
	}

}
