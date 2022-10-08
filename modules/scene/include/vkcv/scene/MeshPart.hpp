#pragma once

#include <vector>

#include <vkcv/Buffer.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/material/Material.hpp>

#include "Bounds.hpp"

namespace vkcv::scene {

    /**
     * @addtogroup vkcv_scene
     * @{
     */
	
	class Scene;
	class Mesh;

    /**
     * A class to represent a group of vertices to render
     * a part of a mesh.
     */
	class MeshPart {
		friend class Mesh;
	
	private:
        /**
         * Parent scene of the mesh part.
         */
		Scene& m_scene;

        /**
         * The vertex data containing its part of the mesh.
         */
		VertexData m_data;

        /**
         * Axis aligned bounding box of the mesh part.
         */
		Bounds m_bounds;

        /**
         * The index of the material used to render
         * this part of the mesh.
         */
		size_t m_materialIndex;

        /**
         * Constructor of a new mesh part with a given scene as parent.
         *
         * @param[in,out] scene Scene
         */
		explicit MeshPart(Scene& scene);

        /**
         * Load vertex and index data from structures provided by the asset loader
         * and add a matching drawcall to the list if the loaded mesh part is valid.
         *
         * @param[in] scene Scene structure from asset loader
         * @param[in] vertexGroup Vertex group structure from asset loader
         * @param[in] types Primitive type order of vertex attributes
         * @param[out] drawcalls List of drawcalls
         */
		void load(const asset::Scene& scene,
				  const asset::VertexGroup& vertexGroup,
				  const std::vector<asset::PrimitiveType>& types,
				  std::vector<InstanceDrawcall>& drawcalls);
	
	public:
        /**
         * Destructor of a mesh part.
         */
		~MeshPart();

        /**
         * Copy-constructor of a mesh part.
         *
         * @param[in] other Other mesh part
         */
		MeshPart(const MeshPart& other);

        /**
         * Move-constructor of a mesh part.
         *
         * @param[in,out] other Other mesh part
         */
        MeshPart(MeshPart&& other) noexcept;

        /**
         * Copy-operator of a mesh part.
         *
         * @param[in] other Other mesh part
         * @return Reference to this mesh part
         */
		MeshPart& operator=(const MeshPart& other);

        /**
         * Move-operator of a mesh part.
         *
         * @param[in,out] other Other mesh part
         * @return Reference to this mesh part
         */
        MeshPart& operator=(MeshPart&& other) noexcept;

        /**
         * Get the material used by this specific part of
         * the mesh for rendering.
         *
         * @return Material
         */
		[[nodiscard]]
		const material::Material& getMaterial() const;

        /**
         * Return the axis aligned bounding box of this
         * specific part of the mesh.
         *
         * @return Axis aligned bounding box of this mesh part
         */
		[[nodiscard]]
		const Bounds& getBounds() const;

        /**
         * Return the status if this part of the mesh is valid
         * as boolean value.
         *
         * @return true if the mesh part is valid, otherwise false
         */
		explicit operator bool() const;

        /**
         * Return the status if this part of the mesh is invalid
         * as boolean value.
         *
         * @return true if the mesh part is invalid, otherwise false
         */
		bool operator!() const;
		
	};

    /** @} */

}
