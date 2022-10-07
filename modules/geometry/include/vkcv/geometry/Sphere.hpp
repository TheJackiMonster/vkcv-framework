#pragma once

#include "Circular.hpp"
#include "Volume.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	/**
	 * A class to use geometry in form of a sphere.
	 */
	class Sphere : public Volume, public Circular {
	public:
		/**
		 * Constructor creating sphere by a given position
		 * as 3D vector and a radius.
		 *
		 * @param[in] position Position of the sphere as 3D vector
		 * @param[in] radius Radius of the sphere
		 */
		Sphere(const glm::vec3& position, float radius);
		
		/**
         * Copy-constructor of a sphere.
         *
         * @param[in] other Other sphere
         */
		Sphere(const Sphere& other) = default;
		
		/**
         * Move-constructor of a sphere.
         *
         * @param[in] other Other sphere
         */
		Sphere(Sphere&& other) = default;
		
		/**
         * Destructor of a sphere.
         */
		~Sphere() = default;
		
		/**
         * Copy-operator of a sphere.
         *
         * @param[in] other Other sphere
         * @return Reference to this sphere
         */
		Sphere& operator=(const Sphere& other) = default;
		
		/**
         * Move-operator of a sphere.
         *
         * @param[in] other Other sphere
         * @return Reference to this sphere
         */
		Sphere& operator=(Sphere&& other) = default;
		
		/**
		 * Returns the signed distance from a point to the closest
		 * surface of the sphere.
		 *
		 * The result is negative if the point is contained by the
		 * sphere.
		 *
		 * @param[in] point Point as 3D vector
		 * @return Signed distance from point to surface
		 */
		[[nodiscard]]
		float distanceTo(const glm::vec3& point) override;
		
		/**
		 * Generates a vertex data structure, which can be
		 * used for rendering via draw calls, containing
		 * the sphere in a discrete form.
		 *
		 * The vertex data will store positions, normals and
		 * UV-coordinates as vertex attributes.
		 *
		 * @param[in,out] core Core instance
		 * @return Vertex data with generated geometry
		 */
		[[nodiscard]]
		VertexData generateVertexData(Core& core) const override;
		
	};
	
	/** @} */

}
