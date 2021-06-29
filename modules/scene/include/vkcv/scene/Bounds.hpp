#pragma once

#include <glm/vec3.hpp>

namespace vkcv::scene {
	
	class Bounds {
	private:
		glm::vec3 m_min;
		glm::vec3 m_max;
		
	public:
		Bounds();
		~Bounds();
		
		Bounds(const Bounds& other) = default;
		Bounds(Bounds&& other) = default;
		
		Bounds& operator=(const Bounds& other) = default;
		Bounds& operator=(Bounds&& other) = default;
		
		void setMin(const glm::vec3& min);
		
		const glm::vec3& getMin() const;
		
		void setMax(const glm::vec3& max);
		
		const glm::vec3& getMax() const;
		
		void setCenter(const glm::vec3& center);
		
		glm::vec3 getCenter() const;
		
		void setSize(const glm::vec3& size);
		
		glm::vec3 getSize() const;
	
	};
	
}
