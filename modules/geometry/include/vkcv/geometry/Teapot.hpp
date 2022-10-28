#pragma once

#include "Geometry.hpp"

namespace vkcv::geometry {

	/**
     * @addtogroup vkcv_geometry
     * @{
     */
	
	/**
	 * A class to use geometry in form of a teapot.
	 */
	class Teapot : public Geometry {
	private:
		/**
		 * Uniform scale of the teapot.
		 */
		float m_scale;
		
	public:
		/**
		 * Constructor creating teapot by a given position
		 * as 3D vector and uniform scale.
		 *
		 * @param[in] position Position of the teapot as 3D vector
		 * @param[in] scale Uniform scale of the teapot
		 */
		explicit Teapot(const glm::vec3& position, float scale);
		
		/**
         * Copy-constructor of a teapot.
         *
         * @param[in] other Other teapot
         */
		Teapot(const Teapot& other) = default;
		
		/**
         * Move-constructor of a teapot.
         *
         * @param[in] other Other teapot
         */
		Teapot(Teapot&& other) = default;
		
		/**
         * Destructor of a teapot.
         */
		~Teapot() = default;
		
		/**
         * Copy-operator of a teapot.
         *
         * @param[in] other Other teapot
         * @return Reference to this teapot
         */
		Teapot& operator=(const Teapot& other) = default;
		
		/**
         * Move-operator of a teapot.
         *
         * @param[in] other Other teapot
         * @return Reference to this teapot
         */
		Teapot& operator=(Teapot&& other) = default;
		
		/**
		 * Return the uniform scale of a teapot.
		 *
		 * @return Uniform scale of the teapot
		 */
		[[nodiscard]]
		float getScale() const;
		
		/**
		 * Set the uniform scale of a teapot.
		 *
		 * @param scale Uniform scale
		 */
		void setScale(float scale);
		
		/**
		 * Generates a vertex data structure, which can be
		 * used for rendering via draw calls, containing
		 * the teapot in a discrete form.
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
