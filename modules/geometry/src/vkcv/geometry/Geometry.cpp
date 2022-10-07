
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
	
}
