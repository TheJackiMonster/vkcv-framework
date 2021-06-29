
#include "vkcv/scene/Node.hpp"

namespace vkcv::scene {
	
	Node::Node(Scene* scene) :
	m_scene(scene) {}
	
	void Node::loadMesh(const asset::Scene &asset_scene, const asset::Mesh &asset_mesh) {
		Mesh mesh (m_scene);
		mesh.load(asset_scene, asset_mesh);
		m_meshes.push_back(mesh);
	}
	
	Node& Node::addNode() {
		Node node (this->m_scene);
		m_nodes.push_back(node);
		return m_nodes.back();
	}

}
