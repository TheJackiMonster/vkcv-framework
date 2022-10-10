
#include "vkcv/geometry/Geometry.hpp"

namespace vkcv::geometry {
	
	Geometry::Geometry(const glm::vec3 &position)
	: m_position(position) {}
	
	const glm::vec3 &Geometry::getPosition() const {
		return m_position;
	}
	
	void Geometry::setPosition(const glm::vec3 &position) {
		m_position = position;
	}
	
	glm::vec3 Geometry::generateTangent(const std::array<glm::vec3, 3>& positions,
										const  std::array<glm::vec2, 3>& uvs) const {
		auto delta1 = positions[1] - positions[0];
		auto delta2 = positions[2] - positions[0];
		
		auto deltaUV1 = uvs[1] - uvs[0];
		auto deltaUV2 = uvs[2] - uvs[0];
		
		return glm::normalize(glm::vec3(
				delta1 * deltaUV2.y -
				delta2 * deltaUV1.y
		));
	}
	
}
