#pragma once

#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/VertexData.hpp>

#include <glm/glm.hpp>

namespace vkcv::geometry {
	
	/**
     * @defgroup vkcv_geometry Geometry Module
     * A module to use basic geometry for rendering.
     * @{
     */
	
	/**
	 * A basic class to provide attributes for any kind of geometry.
	 */
	class Geometry {
	private:
		/**
		 * Position of the geometry in 3D-coordinates.
		 */
		glm::vec3 m_position;
		
	protected:
		/**
		 * Generate tangent from positions and uv-coordinates
		 * for a given triangle.
		 *
		 * @param[in] positions Array of positions
		 * @param[in] uvs Array of uv-coordinates
		 * @return Calculated tangent
		 */
		[[nodiscard]]
		glm::vec3 generateTangent(const std::array<glm::vec3, 3>& positions,
								  const  std::array<glm::vec2, 3>& uvs) const;
		
	public:
		/**
		 * Constructor creating geometry by a given position
		 * as 3D vector.
		 *
		 * @param[in] position Position of the geometry as 3D vector
		 */
		explicit Geometry(const glm::vec3& position);
		
		/**
         * Copy-constructor of a geometry.
         *
         * @param[in] other Other geometry
         */
		Geometry(const Geometry& other) = default;
		
		/**
         * Move-constructor of a geometry.
         *
         * @param[in] other Other geometry
         */
		Geometry(Geometry&& other) = default;
		
		/**
         * Destructor of a geometry.
         */
		~Geometry() = default;
		
		/**
         * Copy-operator of a geometry.
         *
         * @param[in] other Other geometry
         * @return Reference to this geometry
         */
		Geometry& operator=(const Geometry& other) = default;
		
		/**
         * Move-operator of a geometry.
         *
         * @param[in] other Other geometry
         * @return Reference to this geometry
         */
		Geometry& operator=(Geometry&& other) = default;
		
		/**
		 * Return the position of a geometry as 3D vector.
		 *
		 * @return Position of the geometry as 3D vector
		 */
		[[nodiscard]]
		const glm::vec3& getPosition() const;
		
		/**
		 * Set the position of a geometry to a specific
		 * 3D vector.
		 *
		 * @param[in] position Position as 3D vector
		 */
		void setPosition(const glm::vec3& position);
		
		/**
		 * Generates a vertex data structure, which can be
		 * used for rendering via draw calls, containing
		 * the geometry in a discrete form.
		 *
		 * The vertex data will store positions, normals and
		 * UV-coordinates as vertex attributes.
		 *
		 * @param[in,out] core Core instance
		 * @return Vertex data with generated geometry
		 */
		[[nodiscard]]
		virtual VertexData generateVertexData(Core& core) const = 0;
		
		/**
		 * Extracts a geometry data structure from its own
		 * generated vertex data, which can be used for
		 * building bottom level acceleration structures.
		 *
		 * @param[in, out] vertexData Vertex data with generated geometry
		 * @return Geometry data from generated vertex data
		 */
		[[nodiscard]]
		virtual GeometryData extractGeometryData(const VertexData &vertexData) const;
		
	};
	
	/** @} */

}
