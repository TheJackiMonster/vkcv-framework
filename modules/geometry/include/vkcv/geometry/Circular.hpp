#pragma once

#include <cstdlib>

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	/**
	 * A basic class to provide attributes for circular geometry.
	 */
	class Circular {
	private:
		/**
		 * Radius of the circular part of the geometry.
		 */
		float m_radius;
		
		/**
		 * Resolution in case of generating the geometry in a
		 * discrete way.
		 */
		size_t m_resolution;
	
	public:
		/**
		 * Constructor creating circular geometry by a given
		 * radius and a resolution also provides a default.
		 *
		 * @param[in] radius Radius of the circular geometry
		 * @param[in] resoltion Resolution of the circular geometry
		 */
		explicit Circular(float radius, size_t resoltion = 10);
		
		/**
         * Copy-constructor of a circular geometry.
         *
         * @param[in] other Other circular geometry
         */
		Circular(const Circular& other) = default;
		
		/**
         * Move-constructor of a circular geometry.
         *
         * @param[in] other Other circular geometry
         */
		Circular(Circular&& other) = default;
		
		/**
         * Destructor of a circular geometry.
         */
		~Circular() = default;
		
		/**
         * Copy-operator of a circular geometry.
         *
         * @param[in] other Other circular geometry
         * @return Reference to this circular geometry
         */
		Circular& operator=(const Circular& other) = default;
		
		/**
         * Move-operator of a circular geometry.
         *
         * @param[in] other Other circular geometry
         * @return Reference to this circular geometry
         */
		Circular& operator=(Circular&& other) = default;
		
		/**
		 * Return the radius of the circular part of the geometry.
		 *
		 * @return Radius of the circular geometry
		 */
		[[nodiscard]]
		float getRadius() const;
		
		/**
		 * Set the radius of the circular part of the geometry.
		 *
		 * @param[in] radius Radius of the circular geometry
		 */
		void setRadius(float radius);
		
		/**
		 * Return the resolution of the geometry for discrete
		 * generation.
		 *
		 * @return Resolution of the circular geometry
		 */
		[[nodiscard]]
		size_t getResolution() const;
		
		/**
		 * Set the resolution of the geometry for any discrete
		 * generation.
		 *
		 * @param[in] resolution Resolution of the circular geometry
		 */
		void setResolution(size_t resolution);
		
	};
	
	/** @} */
	
}