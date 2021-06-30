
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
	
	void Mesh::recordDrawcalls(std::vector<glm::mat4>& matrices,
							   std::vector<DrawcallInfo>& drawcalls) {
		for (const auto& part : m_parts) {
			matrices.push_back(m_transform);
		}
		
		for (const auto& drawcall : m_drawcalls) {
			drawcalls.push_back(drawcall);
		}
	}

}
