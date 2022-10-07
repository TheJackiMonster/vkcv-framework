#pragma once

#include "Geometry.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	/**
	 * A basic class to provide functions for any volumetric geometry.
	 */
	class Volume : public Geometry {
	private:
	public:
		/**
		 * Constructor creating volumetric geometry by a given
		 * position as 3D vector.
		 *
		 * @param[in] position Position of the geometry as 3D vector
		 */
		explicit Volume(const glm::vec3& position);
		
		/**
         * Copy-constructor of a volumetric geometry.
         *
         * @param[in] other Other volumetric geometry
         */
		Volume(const Volume& other) = default;
		
		/**
         * Move-constructor of a volumetric geometry.
         *
         * @param[in] other Other volumetric geometry
         */
		Volume(Volume&& other) = default;
		
		/**
         * Destructor of a volumetric geometry.
         */
		~Volume() = default;
		
		/**
         * Copy-operator of a volumetric geometry.
         *
         * @param[in] other Other volumetric geometry
         * @return Reference to this volumetric geometry
         */
		Volume& operator=(const Volume& other) = default;
		
		/**
         * Move-operator of a volumetric geometry.
         *
         * @param[in] other Other volumetric geometry
         * @return Reference to this volumetric geometry
         */
		Volume& operator=(Volume&& other) = default;

		/**
		 * Returns the signed distance from a point to the closest
		 * surface of the volumetric geometry.
		 *
		 * The result is negative if the point is contained by the
		 * volumetric geometry.
		 *
		 * @param[in] point Point as 3D vector
		 * @return Signed distance from point to surface
		 */
		[[nodiscard]]
		virtual float distanceTo(const glm::vec3& point) = 0;
		
		/**
		 * Returns as boolean value whether a point is contained
		 * by a volumetric geometry.
		 *
		 * @param[in] point Point as 3D vector
		 * @return true if the point is contained, other false.
		 */
		[[nodiscard]]
		bool contains(const glm::vec3& point);
		
	};
	
	/** @} */
	
}
