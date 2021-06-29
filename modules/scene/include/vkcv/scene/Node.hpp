#pragma once

#include <vector>

#include "Mesh.hpp"

namespace vkcv::scene {
	
	class Scene;
	
	class Node {
		friend class Scene;
		
	private:
		Scene* m_scene;
		
		std::vector<Mesh> m_meshes;
		std::vector<Node> m_nodes;
		
		explicit Node(Scene* scene);
		
		void loadMesh(const asset::Scene& asset_scene, const asset::Mesh& asset_mesh);
	
	public:
		~Node() = default;
		
		Node(const Node& other) = default;
		Node(Node&& other) = default;
		
		Node& operator=(const Node& other) = default;
		Node& operator=(Node&& other) = default;
		
		Node& addNode();
		
	};
	
}
