
#include "vkcv/geometry/Circular.hpp"

namespace vkcv::geometry {
	
	Circular::Circular(float radius)
	: m_radius(radius) {}
	
	float Circular::getRadius() const {
		return m_radius;
	}
	
	void Circular::setRadius(float radius) {
		m_radius = radius;
	}

}
