
#include "vkcv/geometry/Cylinder.hpp"

namespace vkcv::geometry {
	
	Cylinder::Cylinder(const glm::vec3 &position, float height, float radius)
	: Volume(position), Circular(radius), m_height(height) {}
	
	float Cylinder::getHeight() const {
		return m_height;
	}
	
	void Cylinder::setHeight(float height) {
		m_height = height;
	}
	
	float Cylinder::distanceTo(const glm::vec3 &point) {
		const auto& position = getPosition();
		
		const auto verticalDistance = glm::abs(position.y - point.y) - getHeight();
		const auto circularDistance = glm::distance(
				glm::vec2(position.x, position.z),
				glm::vec2(point.x, point.z)
		) - getRadius();
		
		if (circularDistance <= 0.0f) {
			return glm::max(verticalDistance, circularDistance);
		} else
		if (verticalDistance <= 0.0f) {
			return circularDistance;
		} else {
			return glm::length(glm::vec2(verticalDistance, circularDistance));
		}
	}
	
}
