
#include "vkcv/geometry/Cuboid.hpp"

namespace vkcv::geometry {
	
	Cuboid::Cuboid(const glm::vec3 &position, const glm::vec3 &size)
	: Volume(position), m_size(size) {}
	
	Cuboid::Cuboid(const glm::vec3 &position, float size)
	: Cuboid(position, glm::vec3(size)) {}
	
	const glm::vec3 &Cuboid::getSize() const {
		return m_size;
	}
	
	void Cuboid::setSize(const glm::vec3 &size) {
		m_size = size;
	}
	
	float Cuboid::distanceTo(const glm::vec3 &point) {
		const auto offset = (point - getPosition());
		const auto distance = (glm::abs(offset) - getSize() * 0.5f);
		const auto inside = glm::lessThanEqual(distance, glm::vec3(0.0f));
		
		if (glm::all(inside)) {
			return glm::max(glm::max(distance.x, distance.y), distance.z);
		} else {
			return glm::length(glm::vec3(glm::not_(inside)) * distance);
		}
	}
	
}
