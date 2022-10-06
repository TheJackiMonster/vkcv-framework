#pragma once

#include "Volume.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	class Sphere : public Volume {
	private:
		float m_radius;
		
	public:
		Sphere(const glm::vec3& position, float radius);
	
		Sphere(const Sphere& other) = default;
		Sphere(Sphere&& other) = default;
	
		~Sphere() = default;
		
		Sphere& operator=(const Sphere& other) = default;
		Sphere& operator=(Sphere&& other) = default;
		
		[[nodiscard]]
		float getRadius() const;
		
		void setRadius(float radius);
		
		[[nodiscard]]
		float distanceTo(const glm::vec3& point) override;
		
	};
	
	/** @} */

}
