
#include "vkcv/scene/Bounds.hpp"

namespace vkcv::scene {
	
	Bounds::Bounds() :
	m_min(glm::vec3(0)),
	m_max(glm::vec3(0)) {}
	
	Bounds::Bounds(const glm::vec3 &point) :
	m_min(point),
	m_max(point)
	{}
	
	Bounds::Bounds(const glm::vec3 &min, const glm::vec3 &max) :
	m_min(min),
	m_max(max)
	{}

	void Bounds::setMin(const glm::vec3 &min) {
		m_min = min;
	}
	
	const glm::vec3 & Bounds::getMin() const {
		return m_min;
	}
	
	void Bounds::setMax(const glm::vec3 &max) {
		m_max = max;
	}
	
	const glm::vec3 & Bounds::getMax() const {
		return m_max;
	}
	
	void Bounds::setCenter(const glm::vec3 &center) {
		const glm::vec3 size = getSize();
		m_min = center - size / 2.0f;
		m_max = center + size / 2.0f;
	}
	
	glm::vec3 Bounds::getCenter() const {
		return (m_min + m_max) / 2.0f;
	}
	
	void Bounds::setSize(const glm::vec3 &size) {
		const glm::vec3 center = getCenter();
		m_min = center - size / 2.0f;
		m_max = center + size / 2.0f;
	}
	
	glm::vec3 Bounds::getSize() const {
		return (m_max - m_min);
	}
	
	std::array<glm::vec3, 8> Bounds::getCorners() const {
		return {
			m_min,
			glm::vec3(m_min[0], m_min[1], m_max[2]),
			glm::vec3(m_min[0], m_max[1], m_min[2]),
			glm::vec3(m_min[0], m_max[1], m_max[2]),
			glm::vec3(m_max[0], m_min[1], m_min[2]),
			glm::vec3(m_max[0], m_min[1], m_max[2]),
			glm::vec3(m_max[0], m_max[1], m_min[2]),
			m_max
		};
	}
	
	void Bounds::extend(const glm::vec3 &point) {
		m_min = glm::vec3(
				std::min(m_min[0], point[0]),
				std::min(m_min[1], point[1]),
				std::min(m_min[2], point[2])
		);
		
		m_max = glm::vec3(
				std::max(m_max[0], point[0]),
				std::max(m_max[1], point[1]),
				std::max(m_max[2], point[2])
		);
	}
	
	bool Bounds::contains(const glm::vec3 &point) const {
		return (
				(point[0] >= m_min[0]) && (point[0] <= m_max[0]) &&
				(point[1] >= m_min[1]) && (point[1] <= m_max[1]) &&
				(point[2] >= m_min[2]) && (point[2] <= m_max[2])
		);
	}
	
	bool Bounds::contains(const Bounds &other) const {
		return (
				(other.m_min[0] >= m_min[0]) && (other.m_max[0] <= m_max[0]) &&
				(other.m_min[1] >= m_min[1]) && (other.m_max[1] <= m_max[1]) &&
				(other.m_min[2] >= m_min[2]) && (other.m_max[2] <= m_max[2])
		);
	}
	
	bool Bounds::intersects(const Bounds &other) const {
		return (
				(other.m_max[0] >= m_min[0]) && (other.m_min[0] <= m_max[0]) &&
				(other.m_max[1] >= m_min[1]) && (other.m_min[1] <= m_max[1]) &&
				(other.m_max[2] >= m_min[2]) && (other.m_min[2] <= m_max[2])
		);
	}
	
	Bounds::operator bool() const {
		return (
				(m_min[0] <= m_max[0]) &&
				(m_min[1] <= m_max[1]) &&
				(m_min[2] <= m_max[2])
		);
	}
	
	bool Bounds::operator!() const {
		return (
				(m_min[0] > m_max[0]) ||
				(m_min[1] > m_max[1]) ||
				(m_min[2] > m_max[2])
		);
	}
	
	std::ostream& operator << (std::ostream& out, const Bounds& bounds) {
		const auto& min = bounds.getMin();
		const auto& max = bounds.getMax();
		
		return out << "[Bounds: (" << min[0] << ", " << min[1] << ", " << min[2] << ") ("
								   << max[0] << ", " << max[1] << ", " << max[2] << ") ]";
	}
	
}
