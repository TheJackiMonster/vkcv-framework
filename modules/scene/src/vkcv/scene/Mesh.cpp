
#include "vkcv/scene/Mesh.hpp"
#include "vkcv/scene/Scene.hpp"

namespace vkcv::scene {
	
	Mesh::Mesh(Scene* scene) :
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
		m_drawcalls.clear();
		
		for (const auto& vertexGroupIndex : mesh.vertexGroups) {
			if ((vertexGroupIndex < 0) || (vertexGroupIndex >= scene.vertexGroups.size())) {
				continue;
			}
			
			MeshPart part (m_scene);
			part.load(scene, scene.vertexGroups[vertexGroupIndex], m_drawcalls);
			
			if (!part) {
				continue;
			}
			
			m_parts.push_back(part);
		}
		
		m_transform = arrayTo4x4Matrix(mesh.modelMatrix);
	}
	
	Mesh::~Mesh() {
		m_drawcalls.clear();
		m_parts.clear();
	}
	
	Mesh::Mesh(const Mesh &other) :
	m_scene(other.m_scene),
	m_parts(other.m_parts),
	m_drawcalls(other.m_drawcalls),
	m_transform(other.m_transform) {}
	
	Mesh::Mesh(Mesh &&other) noexcept :
	m_scene(other.m_scene),
	m_parts(other.m_parts),
	m_drawcalls(other.m_drawcalls),
	m_transform(other.m_transform) {}
	
	Mesh &Mesh::operator=(const Mesh &other) {
		if (&other == this) {
			return *this;
		}
		
		m_scene = other.m_scene;
		m_parts = std::vector<MeshPart>(other.m_parts);
		m_drawcalls = std::vector<DrawcallInfo>(other.m_drawcalls);
		m_transform = other.m_transform;
		
		return *this;
	}
	
	Mesh &Mesh::operator=(Mesh &&other) noexcept {
		m_scene = other.m_scene;
		m_parts = std::move(other.m_parts);
		m_drawcalls = std::move(other.m_drawcalls);
		m_transform = other.m_transform;
		
		return *this;
	}
	
	static glm::vec3 projectPoint(const glm::mat4& transform, const glm::vec3& point, bool& negative_w) {
		const glm::vec4 position = transform * glm::vec4(point, 1.0f);
		const float perspective = std::abs(position[3]);
		
		/*
		 * We divide by the absolute of the 4th coorditnate because
		 * clipping is weird and points have to move to the other
		 * side of the camera.
		 *
		 * We also need to collect if the 4th coordinate was negative
		 * to know if all corners are behind the camera. So these can
		 * be culled as well
		 */
		negative_w = (position[3] < 0.0f);
		
		return glm::vec3(
				position[0] / perspective,
				position[1] / perspective,
				position[2] / perspective
		);
	}
	
	static bool checkFrustum(const Bounds& bounds) {
		static Bounds frustum (
				glm::vec3(-1.0f, -1.0f, -0.0f),
				glm::vec3(+1.0f, +1.0f, +1.0f)
		);
		
		return bounds.intersects(frustum);
	}
	
	void Mesh::recordDrawcalls(const glm::mat4& viewProjection,
							   std::vector<glm::mat4>& matrices,
							   std::vector<DrawcallInfo>& drawcalls) {
		const glm::mat4 transform = viewProjection * m_transform;
		
		for (size_t i = 0; i < m_parts.size(); i++) {
			const MeshPart& part = m_parts[i];
			const Bounds& bounds = part.getBounds();
			const auto corners = bounds.getCorners();
			
			bool negative_w;
			auto projected = projectPoint(transform, corners[0], negative_w);
			
			Bounds aabb (projected, projected);
			
			for (size_t j = 1; j < corners.size(); j++) {
				bool flag_w;
				projected = projectPoint(transform, corners[j], flag_w);
				aabb.extend(projected);
				negative_w &= flag_w;
			}
			
			if ((negative_w) || (!checkFrustum(aabb))) {
				continue;
			}
			
			matrices.push_back(transform);
			drawcalls.push_back(m_drawcalls[i]);
		}
	}
	
	size_t Mesh::getDrawcallCount() const {
		return m_drawcalls.size();
	}

}
