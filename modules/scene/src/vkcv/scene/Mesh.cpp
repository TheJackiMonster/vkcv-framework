
#include "vkcv/scene/Mesh.hpp"

namespace vkcv::scene {
	
	void MeshPart::load(const asset::Scene& scene,
						const asset::VertexGroup &vertexGroup) {
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
			const asset::Material& material = scene.materials[vertexGroup.materialIndex];
			
			
		}
	}

}
