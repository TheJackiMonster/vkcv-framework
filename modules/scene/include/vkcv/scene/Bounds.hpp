#pragma once

#include <array>
#include <iostream>
#include <glm/vec3.hpp>

namespace vkcv::scene {

    /**
     * @addtogroup vkcv_scene
     * @{
     */
	
	class Bounds {
	private:
		glm::vec3 m_min;
		glm::vec3 m_max;
		
	public:
		Bounds();
		Bounds(const glm::vec3& min, const glm::vec3& max);
		~Bounds() = default;
		
		Bounds(const Bounds& other) = default;
		Bounds(Bounds&& other) = default;
		
		Bounds& operator=(const Bounds& other) = default;
		Bounds& operator=(Bounds&& other) = default;
		
		void setMin(const glm::vec3& min);
		
		[[nodiscard]]
		const glm::vec3& getMin() const;
		
		void setMax(const glm::vec3& max);
		
		[[nodiscard]]
		const glm::vec3& getMax() const;
		
		void setCenter(const glm::vec3& center);
		
		[[nodiscard]]
		glm::vec3 getCenter() const;
		
		void setSize(const glm::vec3& size);
		
		[[nodiscard]]
		glm::vec3 getSize() const;
		
		[[nodiscard]]
		std::array<glm::vec3, 8> getCorners() const;
		
		void extend(const glm::vec3& point);
		
		[[nodiscard]]
		bool contains(const glm::vec3& point) const;
		
		[[nodiscard]]
		bool contains(const Bounds& other) const;
		
		[[nodiscard]]
		bool intersects(const Bounds& other) const;
		
		[[nodiscard]]
		explicit operator bool() const;
		
		[[nodiscard]]
		bool operator!() const;
	
	};
	
	std::ostream& operator << (std::ostream& out, const Bounds& bounds);

    /** @} */
	
}
