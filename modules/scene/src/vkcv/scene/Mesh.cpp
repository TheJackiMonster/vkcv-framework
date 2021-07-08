
#include "vkcv/scene/Mesh.hpp"
#include "vkcv/scene/Scene.hpp"
#include "Frustum.hpp"

namespace vkcv::scene {
	
	Mesh::Mesh(Scene& scene) :
	m_scene(scene) {}
	
	static glm::mat4 arrayTo4x4Matrix(const std::array<float,16>& array){
		glm::mat4 matrix;
		
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				matrix[i][j] = array[j * 4 + i];
			}
		}
		
		return matrix;
	}
	
	void Mesh::load(const asset::Scene &scene, const asset::Mesh &mesh) {
		m_parts.clear();
		m_drawcalls.clear();
		
		m_transform = arrayTo4x4Matrix(mesh.modelMatrix);
		
		for (const auto& vertexGroupIndex : mesh.vertexGroups) {
			if ((vertexGroupIndex < 0) || (vertexGroupIndex >= scene.vertexGroups.size())) {
				continue;
			}
			
			MeshPart part (m_scene);
			part.load(scene, scene.vertexGroups[vertexGroupIndex], m_drawcalls);
			
			if (!part) {
				continue;
			}
			
			auto bounds = transformBounds(m_transform, part.getBounds());
			
			if (m_parts.empty()) {
				m_bounds = bounds;
			} else {
				m_bounds.extend(bounds.getMin());
				m_bounds.extend(bounds.getMax());
			}
			
			m_parts.push_back(part);
		}
	}
	
	Mesh::~Mesh() {
		m_drawcalls.clear();
		m_parts.clear();
	}
	
	Mesh &Mesh::operator=(const Mesh &other) {
		if (&other == this) {
			return *this;
		}
		
		m_parts.resize(other.m_parts.size(), MeshPart(m_scene));
		
		for (size_t i = 0; i < m_parts.size(); i++) {
			m_parts[i] = other.m_parts[i];
		}
		
		m_drawcalls = std::vector<DrawcallInfo>(other.m_drawcalls);
		m_transform = other.m_transform;
		m_bounds = other.m_bounds;
		
		return *this;
	}
	
	Mesh &Mesh::operator=(Mesh &&other) noexcept {
		m_parts.resize(other.m_parts.size(), MeshPart(m_scene));
		
		for (size_t i = 0; i < m_parts.size(); i++) {
			m_parts[i] = std::move(other.m_parts[i]);
		}
		
		m_drawcalls = std::move(other.m_drawcalls);
		m_transform = other.m_transform;
		m_bounds = other.m_bounds;
		
		return *this;
	}
	
	void Mesh::recordDrawcalls(const glm::mat4& viewProjection,
							   PushConstants& pushConstants,
							   std::vector<DrawcallInfo>& drawcalls,
							   const RecordMeshDrawcallFunction& record) {
		const glm::mat4 transform = viewProjection * m_transform;
		
		if (!checkFrustum(viewProjection, m_bounds)) {
			return;
		}
		
		if (m_drawcalls.size() == 1) {
			drawcalls.push_back(m_drawcalls[0]);
			
			if (record) {
				record(transform, m_transform, pushConstants, drawcalls.back());
			}
		} else {
			for (size_t i = 0; i < m_parts.size(); i++) {
				const MeshPart& part = m_parts[i];
				
				if (!checkFrustum(transform, part.getBounds())) {
					continue;
				}
				
				drawcalls.push_back(m_drawcalls[i]);
				
				if (record) {
					record(transform, m_transform, pushConstants, drawcalls.back());
				}
			}
		}
	}
	
	size_t Mesh::getDrawcallCount() const {
		return m_drawcalls.size();
	}
	
	const Bounds& Mesh::getBounds() const {
		return m_bounds;
	}

}
