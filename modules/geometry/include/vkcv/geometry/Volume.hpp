#pragma once

#include "Geometry.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	class Volume : public Geometry {
	private:
	public:
		explicit Volume(const glm::vec3& position);
		
		Volume(const Volume& other) = default;
		Volume(Volume&& other) = default;
		
		~Volume() = default;
		
		Volume& operator=(const Volume& other) = default;
		Volume& operator=(Volume&& other) = default;

		[[nodiscard]]
		virtual float distanceTo(const glm::vec3& point) = 0;
		
		[[nodiscard]]
		bool contains(const glm::vec3& point);
		
	};
	
	/** @} */
	
}
