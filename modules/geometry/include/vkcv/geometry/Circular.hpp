#pragma once

#include <cstdlib>

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	class Circular {
	private:
		float m_radius;
		size_t m_resolution;
	
	public:
		explicit Circular(float radius, size_t resoltion = 10);
		
		Circular(const Circular& other) = default;
		Circular(Circular&& other) = default;
		
		~Circular() = default;
		
		Circular& operator=(const Circular& other) = default;
		Circular& operator=(Circular&& other) = default;
		
		[[nodiscard]]
		float getRadius() const;
		
		void setRadius(float radius);
		
		[[nodiscard]]
		size_t getResolution() const;
		
		void setResolution(size_t resolution);
		
	};
	
	/** @} */
	
}