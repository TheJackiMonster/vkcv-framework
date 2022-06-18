
#include "vkcv/scene/MeshPart.hpp"
#include "vkcv/scene/Scene.hpp"

namespace vkcv::scene {
	
	struct vertex_t {
		float positionU [4];
		float normalV [4];
	};
	
	MeshPart::MeshPart(Scene& scene) :
	m_scene(scene),
	m_vertices(),
	m_indices(),
	m_indexCount(0),
	m_descriptorSet(),
	m_bounds(),
	m_materialIndex(std::numeric_limits<size_t>::max()) {}
	
	void MeshPart::load(const asset::Scene& scene,
						const asset::VertexGroup &vertexGroup,
						std::vector<DrawcallInfo>& drawcalls) {
		Core& core = *(m_scene.m_core);
		
		auto vertexBuffer = core.createBuffer<vertex_t>(
				BufferType::STORAGE, vertexGroup.numVertices
		);
		
		std::vector<vertex_t> vertices;
		vertices.reserve(vertexBuffer.getCount());
		
		for (const auto& attribute : vertexGroup.vertexBuffer.attributes) {
			if (attribute.componentType != vkcv::asset::ComponentType::FLOAT32) {
				continue;
			}
			
			size_t offset = attribute.offset;
			
			for (size_t i = 0; i < vertexBuffer.getCount(); i++) {
				const auto *data = reinterpret_cast<const float*>(
						vertexGroup.vertexBuffer.data.data() + offset
				);
				
				switch (attribute.type) {
					case vkcv::asset::PrimitiveType::POSITION:
						memcpy(vertices[i].positionU, data, sizeof(float) * attribute.componentCount);
						break;
					case vkcv::asset::PrimitiveType::NORMAL:
						memcpy(vertices[i].normalV, data, sizeof(float) * attribute.componentCount);
						break;
					case vkcv::asset::PrimitiveType::TEXCOORD_0:
						if (attribute.componentCount != 2) {
							break;
						}
						
						vertices[i].positionU[3] = data[0];
						vertices[i].normalV[3] = data[1];
						break;
					default:
						break;
				}
				
				offset += attribute.stride;
			}
		}
		
		vertexBuffer.fill(vertices);
		m_vertices = vertexBuffer.getHandle();
		
		auto indexBuffer = core.createBuffer<uint8_t>(
				BufferType::INDEX, vertexGroup.indexBuffer.data.size()
		);
		
		indexBuffer.fill(vertexGroup.indexBuffer.data);
		m_indices = indexBuffer.getHandle();
		m_indexCount = vertexGroup.numIndices;
		
		m_descriptorSet = core.createDescriptorSet(m_scene.getDescriptorSetLayout());
		
		DescriptorWrites writes;
		writes.writeStorageBuffer(0, m_vertices);
		core.writeDescriptorSet(m_descriptorSet, writes);
		
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
			
			if (!getMaterial()) {
				m_scene.loadMaterial(m_materialIndex, scene, scene.materials[vertexGroup.materialIndex]);
			}
			
			m_scene.increaseMaterialUsage(m_materialIndex);
		} else {
			m_materialIndex = std::numeric_limits<size_t>::max();
		}
		
		if (*this) {
			const auto& material = getMaterial();
			
			IndexBitCount indexBitCount;
			
			switch (vertexGroup.indexBuffer.type) {
				case asset::IndexType::UINT8:
					indexBitCount = IndexBitCount::Bit8;
					break;
				case asset::IndexType::UINT16:
					indexBitCount = IndexBitCount::Bit16;
					break;
				case asset::IndexType::UINT32:
					indexBitCount = IndexBitCount::Bit32;
					break;
				default:
					indexBitCount = IndexBitCount::Bit16;
					vkcv_log(LogLevel::WARNING, "Unsupported index type!");
					break;
			}
			
			drawcalls.push_back(DrawcallInfo(
					vkcv::Mesh(indexBuffer.getVulkanHandle(), m_indexCount, indexBitCount),
					{
						DescriptorSetUsage(0, m_descriptorSet),
						DescriptorSetUsage(1, material.getDescriptorSet())
					}
			));
		}
	}
	
	MeshPart::~MeshPart() {
		m_scene.decreaseMaterialUsage(m_materialIndex);
	}
	
	MeshPart::MeshPart(const MeshPart &other) :
			m_scene(other.m_scene),
			m_vertices(other.m_vertices),
			m_indices(other.m_indices),
			m_indexCount(other.m_indexCount),
			m_descriptorSet(other.m_descriptorSet),
			m_bounds(other.m_bounds),
			m_materialIndex(other.m_materialIndex) {
		m_scene.increaseMaterialUsage(m_materialIndex);
	}
	
	MeshPart::MeshPart(MeshPart &&other) noexcept :
			m_scene(other.m_scene),
			m_vertices(other.m_vertices),
			m_indices(other.m_indices),
			m_indexCount(other.m_indexCount),
			m_descriptorSet(other.m_descriptorSet),
			m_bounds(other.m_bounds),
			m_materialIndex(other.m_materialIndex) {
		m_scene.increaseMaterialUsage(m_materialIndex);
	}
	
	MeshPart &MeshPart::operator=(const MeshPart &other) {
		if (&other == this) {
			return *this;
		}
		
		m_vertices = other.m_vertices;
		m_indices = other.m_indices;
		m_indexCount = other.m_indexCount;
		m_descriptorSet = other.m_descriptorSet;
		m_bounds = other.m_bounds;
		m_materialIndex = other.m_materialIndex;
		
		return *this;
	}
	
	MeshPart &MeshPart::operator=(MeshPart &&other) noexcept {
		m_vertices = other.m_vertices;
		m_indices = other.m_indices;
		m_indexCount = other.m_indexCount;
		m_descriptorSet = other.m_descriptorSet;
		m_bounds = other.m_bounds;
		m_materialIndex = other.m_materialIndex;
		
		return *this;
	}
	
	const material::Material & MeshPart::getMaterial() const {
		return m_scene.getMaterial(m_materialIndex);
	}
	
	MeshPart::operator bool() const {
		return (
				(getMaterial()) &&
				(m_vertices) &&
				(m_indices)
		);
	}
	
	bool MeshPart::operator!() const {
		return (
				(!getMaterial()) ||
				(!m_vertices) ||
				(!m_indices)
		);
	}
	
	const Bounds &MeshPart::getBounds() const {
		return m_bounds;
	}
	
}
