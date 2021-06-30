
#include "vkcv/scene/Bounds.hpp"

namespace vkcv::scene {
	
	Bounds::Bounds() :
	m_min(glm::vec3(0)),
	m_max(glm::vec3(0)) {}

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

}
