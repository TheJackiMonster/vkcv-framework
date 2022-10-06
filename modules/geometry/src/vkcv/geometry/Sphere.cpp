
#include "vkcv/geometry/Sphere.hpp"

namespace vkcv::geometry {
	
	Sphere::Sphere(const glm::vec3& position, float radius)
	: Volume(position), m_radius(radius) {}
	
	float Sphere::getRadius() const {
		return m_radius;
	}
	
	void Sphere::setRadius(float radius) {
		m_radius = radius;
	}
	
	float Sphere::distanceTo(const glm::vec3 &point) {
		return glm::distance(getPosition(), point) - getRadius();
	}

}
