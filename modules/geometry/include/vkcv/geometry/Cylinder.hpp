#pragma once

#include "Circular.hpp"
#include "Volume.hpp"

namespace vkcv::geometry {
	
	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	/**
	 * A class to use geometry in form of a cylinder.
	 */
	class Cylinder : public Volume, public Circular {
	private:
		/**
		 * Height of the cylinder (Y-axis)
		 */
		float m_height;
	
	public:
		/**
		 * Constructor creating cylinder by a given position
		 * as 3D vector, a given height and a radius.
		 *
		 * @param[in] position Position of the cylinder as 3D vector
		 * @param[in] height Height of the cylinder
		 * @param[in] radius Radius of the cylinder
		 */
		Cylinder(const glm::vec3& position, float height, float radius);
		
		/**
         * Copy-constructor of a cylinder.
         *
         * @param[in] other Other cylinder
         */
		Cylinder(const Cylinder& other) = default;
		
		/**
         * Move-constructor of a cylinder.
         *
         * @param[in] other Other cylinder
         */
		Cylinder(Cylinder&& other) = default;
		
		/**
         * Destructor of a cylinder.
         */
		~Cylinder() = default;
		
		/**
         * Copy-operator of a cylinder.
         *
         * @param[in] other Other cylinder
         * @return Reference to this cylinder
         */
		Cylinder& operator=(const Cylinder& other) = default;
		
		/**
         * Move-operator of a cylinder.
         *
         * @param[in] other Other cylinder
         * @return Reference to this cylinder
         */
		Cylinder& operator=(Cylinder&& other) = default;
		
		/**
		 * Return the height of the cylinder.
		 *
		 * @return Height of the cylinder
		 */
		[[nodiscard]]
		float getHeight() const;
		
		/**
		 * Set the height of the cylinder.
		 *
		 * @param[in] height Height of the cylinder
		 */
		void setHeight(float height);
		
		/**
		 * Returns the signed distance from a point to the closest
		 * surface of the cylinder.
		 *
		 * The result is negative if the point is contained by the
		 * cylinder.
		 *
		 * @param[in] point Point as 3D vector
		 * @return Signed distance from point to surface
		 */
		[[nodiscard]]
		float distanceTo(const glm::vec3& point) override;
		
		/**
		 * Generates a vertex data structure, which can be
		 * used for rendering via draw calls, containing
		 * the cylinder in a discrete form.
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