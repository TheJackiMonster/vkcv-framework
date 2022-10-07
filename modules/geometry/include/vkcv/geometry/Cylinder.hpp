#pragma once

#include "Circular.hpp"
#include "Volume.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	class Cylinder : public Volume, public Circular {
	private:
		float m_height;
	
	public:
		Cylinder(const glm::vec3& position, float height, float radius);
		
		Cylinder(const Cylinder& other) = default;
		Cylinder(Cylinder&& other) = default;
		
		~Cylinder() = default;
		
		Cylinder& operator=(const Cylinder& other) = default;
		Cylinder& operator=(Cylinder&& other) = default;
		
		[[nodiscard]]
		float getHeight() const;
		
		void setHeight(float height);
		
		[[nodiscard]]
		float distanceTo(const glm::vec3& point) override;
		
	};
	
	/** @} */
	
}