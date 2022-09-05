
#include "vkcv/scene/MeshPart.hpp"
#include "vkcv/scene/Scene.hpp"

#include <vkcv/Buffer.hpp>

namespace vkcv::scene {
	
	MeshPart::MeshPart(Scene& scene) :
	m_scene(scene),
	m_vertices(),
	m_vertexBindings(),
	m_indices(),
	m_indexCount(0),
	m_bounds(),
	m_materialIndex(std::numeric_limits<size_t>::max()) {}
	
	void MeshPart::load(const asset::Scene& scene,
						const asset::VertexGroup &vertexGroup,
						const std::vector<asset::PrimitiveType>& types,
						std::vector<InstanceDrawcall>& drawcalls) {
		Core& core = *(m_scene.m_core);
		
		auto vertexBuffer = buffer<uint8_t>(
				core, BufferType::VERTEX, vertexGroup.vertexBuffer.data.size()
		);
		
		vertexBuffer.fill(vertexGroup.vertexBuffer.data);
		m_vertices = vertexBuffer.getHandle();
		
		m_vertexBindings = asset::loadVertexBufferBindings(
				vertexGroup.vertexBuffer.attributes,
				vertexBuffer.getHandle(),
				types
		);
		
		auto indexBuffer = buffer<uint8_t>(
				core, BufferType::INDEX, vertexGroup.indexBuffer.data.size()
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
			
			VertexData vertexData (m_vertexBindings);
			vertexData.setIndexBuffer(indexBuffer.getHandle(), indexBitCount);
			vertexData.setCount(m_indexCount);
			
			InstanceDrawcall drawcall (vertexData);
			drawcall.useDescriptorSet(0, material.getDescriptorSet());
			
			drawcalls.push_back(drawcall);
		}
	}
	
	MeshPart::~MeshPart() {
		m_scene.decreaseMaterialUsage(m_materialIndex);
	}
	
	MeshPart::MeshPart(const MeshPart &other) :
			m_scene(other.m_scene),
			m_vertices(other.m_vertices),
			m_vertexBindings(other.m_vertexBindings),
			m_indices(other.m_indices),
			m_indexCount(other.m_indexCount),
			m_bounds(other.m_bounds),
			m_materialIndex(other.m_materialIndex) {
		m_scene.increaseMaterialUsage(m_materialIndex);
	}
	
	MeshPart::MeshPart(MeshPart &&other) noexcept :
			m_scene(other.m_scene),
			m_vertices(other.m_vertices),
			m_vertexBindings(other.m_vertexBindings),
			m_indices(other.m_indices),
			m_indexCount(other.m_indexCount),
			m_bounds(other.m_bounds),
			m_materialIndex(other.m_materialIndex) {
		m_scene.increaseMaterialUsage(m_materialIndex);
	}
	
	MeshPart &MeshPart::operator=(const MeshPart &other) {
		if (&other == this) {
			return *this;
		}
		
		m_vertices = other.m_vertices;
		m_vertexBindings = other.m_vertexBindings;
		m_indices = other.m_indices;
		m_indexCount = other.m_indexCount;
		m_bounds = other.m_bounds;
		m_materialIndex = other.m_materialIndex;
		
		return *this;
	}
	
	MeshPart &MeshPart::operator=(MeshPart &&other) noexcept {
		m_vertices = other.m_vertices;
		m_vertexBindings = other.m_vertexBindings;
		m_indices = other.m_indices;
		m_indexCount = other.m_indexCount;
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
