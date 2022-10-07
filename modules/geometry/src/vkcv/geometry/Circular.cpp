
#include "vkcv/geometry/Circular.hpp"

namespace vkcv::geometry {
	
	Circular::Circular(float radius, size_t resolution)
	: m_radius(radius), m_resolution(resolution) {}
	
	float Circular::getRadius() const {
		return m_radius;
	}
	
	void Circular::setRadius(float radius) {
		m_radius = radius;
	}
	
	size_t Circular::getResolution() const {
		return m_resolution;
	}
	
	void Circular::setResolution(size_t resolution) {
		m_resolution = resolution;
	}
	
}
