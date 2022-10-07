#pragma once

#include "Circular.hpp"
#include "Volume.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	class Sphere : public Volume, public Circular {
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
		float distanceTo(const glm::vec3& point) override;
		
		[[nodiscard]]
		VertexData generateVertexData(Core& core) const override;
		
	};
	
	/** @} */

}
