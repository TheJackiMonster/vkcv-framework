#pragma once

#include <glm/mat4x4.hpp>

#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/camera/Camera.hpp>

#include "MeshPart.hpp"

namespace vkcv::scene {

    /**
     * @addtogroup vkcv_scene
     * @{
     */

    /**
     * An event function type to be called on per drawcall recording level to adjust data
     * like push constants with provided matrices.
     */
	typedef typename event_function<const glm::mat4&, const glm::mat4&, PushConstants&, vkcv::Drawcall&>::type RecordMeshDrawcallFunction;
	
	/**
	* An event function type to be called on per geometry while creating an
	* acceleration structure using the given instance index and geometry index
	* of each geometry in a bottom-level acceleration structure.
	*/
	typedef typename event_function<size_t, size_t, const vkcv::GeometryData&, const glm::mat4&>::type ProcessGeometryFunction;
	
	class Node;

    /**
     * A class to represent a whole mesh to render.
     */
	class Mesh {
		friend class Node;
		
	private:
        /**
         * Parent scene of the mesh.
         */
		Scene& m_scene;

        /**
         * List of the meshes parts.
         */
		std::vector<MeshPart> m_parts;

        /**
         * List of the meshes drawcalls to render.
         */
		std::vector<InstanceDrawcall> m_drawcalls;

        /**
         * Local transformation matrix of the mesh.
         */
		glm::mat4 m_transform;

        /**
         * Axis aligned bounding box of the mesh.
         */
		Bounds m_bounds;

        /**
         * Constructor of a new mesh with a given scene as parent.
         *
         * @param[in,out] scene Scene
         */
		explicit Mesh(Scene& scene);

        /**
         * Load mesh data from a scene structure from the asset loader
         * creating and loading all mesh parts being required.
         *
         * @param[in] scene Scene structure from asset loader
         * @param[in] mesh Mesh structure from asset loader
         * @param[in] types Primitive type order of vertex attributes
         */
		void load(const asset::Scene& scene,
				  const asset::Mesh& mesh,
				  const std::vector<asset::PrimitiveType>& types);

        /**
         * Record drawcalls of the mesh, equally to all its visible parts.
         *
         * @param[in] viewProjection View-transformation and projection as 4x4 matrix
         * @param[out] pushConstants Structure to store push constants per drawcall
         * @param[out] drawcalls List of drawcall structures
         * @param[in] record Drawcall recording event function
         */
		void recordDrawcalls(const glm::mat4& viewProjection,
							 PushConstants& pushConstants,
							 std::vector<InstanceDrawcall>& drawcalls,
							 const RecordMeshDrawcallFunction& record);
		
		/**
		 * Creates acceleration structures for the whole geometry of this mesh and
		 * appends it to a given list.
		 *
		 * @param[in,out] core Core instance
		 * @param[out] accelerationStructures List of acceleration structures
		 * @param[in] process Geometry processing event function
		 */
		void appendAccelerationStructures(Core& core,
				std::vector<AccelerationStructureHandle>&accelerationStructures,
				const ProcessGeometryFunction &process) const;

        /**
         * Return the amount of drawcalls of the mesh
         * as sum of all its parts.
         *
         * @return Amount of drawcalls
         */
		[[nodiscard]]
		size_t getDrawcallCount() const;
	
	public:
        /**
         * Destructor of a mesh.
         */
		~Mesh();

        /**
         * Copy-constructor of a mesh.
         *
         * @param[in] other Other mesh instance
         */
		Mesh(const Mesh& other) = default;

        /**
         * Move-constructor of a mesh.
         *
         * @param[in,out] other Other mesh instance
         */
        Mesh(Mesh&& other) = default;

        /**
         * Copy-operator of a mesh.
         *
         * @param[in] other Other mesh instance
         * @return Reference to this mesh instance
         */
		Mesh& operator=(const Mesh& other);

        /**
         * Move-operator of a mesh.
         *
         * @param[in,out] other Other mesh instance
         * @return Reference to this mesh instance
         */
		Mesh& operator=(Mesh&& other) noexcept;

        /**
         * Return the axis aligned bounding box of the mesh.
         *
         * @return Axis aligned bounding box of this mesh
         */
		[[nodiscard]]
		const Bounds& getBounds() const;
	
	};

    /** @} */
	
}
