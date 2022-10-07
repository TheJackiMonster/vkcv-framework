#pragma once

#include "Volume.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	/**
	 * A class to use geometry in form of a cuboid.
	 */
	class Cuboid : public Volume {
	private:
		/**
		 * Size of the cuboid in 3D-space.
		 */
		glm::vec3 m_size;
		
	public:
		/**
		 * Constructor creating cuboid by a given position
		 * and size as 3D vectors.
		 *
		 * @param[in] position Position of the cuboid as 3D vector
		 * @param[in] size Size of the cuboid as 3D vector
		 */
		Cuboid(const glm::vec3& position, const glm::vec3& size);
		
		/**
		 * Constructor creating cube by a given position
		 * as 3D vector and uniform size.
		 *
		 * @param[in] position Position of the cube as 3D vector
		 * @param[in] size Uniform size of the cube
		 */
		Cuboid(const glm::vec3& position, float size);
		
		/**
         * Copy-constructor of a cuboid.
         *
         * @param[in] other Other cuboid
         */
		Cuboid(const Cuboid& other) = default;
		
		/**
         * Move-constructor of a cuboid.
         *
         * @param[in] other Other cuboid
         */
		Cuboid(Cuboid&& other) = default;
		
		/**
         * Destructor of a cuboid.
         */
		~Cuboid() = default;
		
		/**
         * Copy-operator of a cuboid.
         *
         * @param[in] other Other cuboid
         * @return Reference to this cuboid
         */
		Cuboid& operator=(const Cuboid& other) = default;
		
		/**
         * Move-operator of a cuboid.
         *
         * @param[in] other Other cuboid
         * @return Reference to this cuboid
         */
		Cuboid& operator=(Cuboid&& other) = default;
		
		/**
		 * Return the size of a cuboid as 3D vector.
		 *
		 * @return Size of the cuboid as 3D vector
		 */
		[[nodiscard]]
		const glm::vec3& getSize() const;
		
		/**
		 * Set the size of a cuboid to a specific
		 * 3D vector.
		 *
		 * @param[in] position Size as 3D vector
		 */
		void setSize(const glm::vec3& size);
		
		/**
		 * Returns the signed distance from a point to the closest
		 * surface of the cuboid.
		 *
		 * The result is negative if the point is contained by the
		 * cuboid.
		 *
		 * @param[in] point Point as 3D vector
		 * @return Signed distance from point to surface
		 */
		[[nodiscard]]
		float distanceTo(const glm::vec3& point) override;
		
		/**
		 * Generates a vertex data structure, which can be
		 * used for rendering via draw calls, containing
		 * the cuboid in a discrete form.
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
