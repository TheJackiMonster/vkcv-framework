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
	
	class Node {
		friend class Scene;
		
	private:
		Scene& m_scene;
		
		std::vector<Mesh> m_meshes;
		std::vector<Node> m_nodes;
		Bounds m_bounds;
		
		explicit Node(Scene& scene);
		
		void addMesh(const Mesh& mesh);
		
		void loadMesh(const asset::Scene& asset_scene, const asset::Mesh& asset_mesh);
		
		void recordDrawcalls(const glm::mat4& viewProjection,
							 PushConstants& pushConstants,
							 std::vector<DrawcallInfo>& drawcalls,
							 const RecordMeshDrawcallFunction& record);
		
		void splitMeshesToSubNodes(size_t maxMeshesPerNode);
		
		[[nodiscard]]
		size_t getDrawcallCount() const;
		
		size_t addNode();
		
		Node& getNode(size_t index);
		
		[[nodiscard]]
		const Node& getNode(size_t index) const;
	
	public:
		~Node();
		
		Node(const Node& other) = default;
		Node(Node&& other) = default;
		
		Node& operator=(const Node& other);
		Node& operator=(Node&& other) noexcept;
		
		[[nodiscard]]
		const Bounds& getBounds() const;
		
	};

    /** @} */
	
}
