#pragma once

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	class Circular {
	private:
		float m_radius;
	
	public:
		explicit Circular(float radius);
		
		Circular(const Circular& other) = default;
		Circular(Circular&& other) = default;
		
		~Circular() = default;
		
		Circular& operator=(const Circular& other) = default;
		Circular& operator=(Circular&& other) = default;
		
		[[nodiscard]]
		float getRadius() const;
		
		void setRadius(float radius);
		
	};
	
	/** @} */
	
}