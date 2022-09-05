#pragma once

#include <vector>

#include <vkcv/camera/Camera.hpp>

#include "Bounds.hpp"
#include "Mesh.hpp"

namespace vkcv::scene {

    /**
     * @addtogroup vkcv_scene
     * @{
     */
	
	class Scene;

    /**
     * A class to represent a graph node in a scene graph.
     */
	class Node {
		friend class Scene;
		
	private:
        /**
         * Parent scene of the node.
         */
		Scene& m_scene;

        /**
         * List of meshes added the node.
         */
		std::vector<Mesh> m_meshes;

        /**
         * List of child nodes added the node.
         */
		std::vector<Node> m_nodes;

        /**
         * Axis aligned bounding box of the node.
         */
		Bounds m_bounds;

        /**
         * Constructor of a new node with a given scene as parent.
         *
         * @param[in,out] scene Scene
         */
		explicit Node(Scene& scene);

        /**
         * Add a given mesh to this node for drawcall recording.
         *
         * @param[in] mesh Mesh
         */
		void addMesh(const Mesh& mesh);

        /**
         * Load and add a mesh from a scene preloaded with the asset loader.
         *
         * @param[in] asset_scene Scene structure from asset loader
         * @param[in] asset_mesh Mesh structure from asset loader
         * @param[in] types Primitive type order of vertex attributes
         */
		void loadMesh(const asset::Scene& asset_scene,
					  const asset::Mesh& asset_mesh,
					  const std::vector<asset::PrimitiveType>& types);

        /**
         * Record drawcalls of all meshes of this node and its child nodes.
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
         * Splits child nodes into tree based graphs of nodes
         * until all nodes contain an amount of meshes below
         * a given maximum.
         *
         * @param[in] maxMeshesPerNode Maximum amount of meshes per node
         */
		void splitMeshesToSubNodes(size_t maxMeshesPerNode);

        /**
         * Return the sum of drawcalls in the graph of this node.
         *
         * @return Amount of drawcalls
         */
		[[nodiscard]]
		size_t getDrawcallCount() const;

        /**
         * Add a new node as child to the scene graph with this node
         * as parent and return its index.
         *
         * @return Index of the new node
         */
		size_t addNode();

        /**
         * Get a reference to the child node with a given index.
         *
         * @param[in] index Valid index of a child node
         * @return Matching child node
         */
		Node& getNode(size_t index);

        /**
         * Get a const reference to the child node with a given index.
         *
         * @param[in] index Valid index of a child node
         * @return Matching child node
         */
		[[nodiscard]]
		const Node& getNode(size_t index) const;
	
	public:
        /**
         * Destructor of a scene node.
         */
		~Node();

        /**
         * Copy-constructor of a scene node.
         *
         * @param[in] other Other scene node
         */
		Node(const Node& other) = default;

        /**
         * Move-constructor of a scene node.
         *
         * @param[in,out] other Other scene node
         */
		Node(Node&& other) = default;

        /**
         * Copy-operator of a scene node.
         *
         * @param[in] other Other scene node
         * @return Reference of this node
         */
		Node& operator=(const Node& other);

        /**
         * Move-operator of a scene node.
         *
         * @param[in,out] other Other scene node
         * @return Reference of this node
         */
		Node& operator=(Node&& other) noexcept;

        /**
         * Return the axis aligned bounding box of the
         * scene node.
         *
         * @return Axis aligned bounding box of this node
         */
		[[nodiscard]]
		const Bounds& getBounds() const;
		
	};

    /** @} */
	
}
