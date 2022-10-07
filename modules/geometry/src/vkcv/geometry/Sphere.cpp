
#include "vkcv/geometry/Sphere.hpp"

namespace vkcv::geometry {
	
	Sphere::Sphere(const glm::vec3& position, float radius)
	: Volume(position), Circular(radius) {}
	
	float Sphere::distanceTo(const glm::vec3 &point) {
		return glm::distance(getPosition(), point) - getRadius();
	}

}
