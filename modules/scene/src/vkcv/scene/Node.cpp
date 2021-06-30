
#include "vkcv/scene/Node.hpp"

namespace vkcv::scene {
	
	Node::Node(Scene* scene) :
	m_scene(scene),
	m_meshes(),
	m_nodes() {}
	
	Node::~Node() {
		m_nodes.clear();
		m_meshes.clear();
	}
	
	Node::Node(const Node &other) :
	m_scene(other.m_scene),
	m_meshes(other.m_meshes),
	m_nodes(other.m_nodes) {}
	
	Node::Node(Node &&other) noexcept :
	m_scene(other.m_scene),
	m_meshes(other.m_meshes),
	m_nodes(other.m_nodes) {}
	
	Node &Node::operator=(const Node &other) {
		if (&other == this) {
			return *this;
		}
		
		m_scene = other.m_scene;
		m_meshes = std::vector<Mesh>(other.m_meshes);
		m_nodes = std::vector<Node>(other.m_nodes);
		
		return *this;
	}
	
	Node &Node::operator=(Node &&other) noexcept {
		m_scene = other.m_scene;
		m_meshes = std::move(other.m_meshes);
		m_nodes = std::move(other.m_nodes);
		
		return *this;
	}
	
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
	
	void Node::recordDrawcalls(std::vector<glm::mat4>& matrices,
							   std::vector<DrawcallInfo>& drawcalls) {
		for (auto& mesh : m_meshes) {
			mesh.recordDrawcalls(matrices, drawcalls);
		}
		
		for (auto& node : m_nodes) {
			node.recordDrawcalls(matrices, drawcalls);
		}
	}

}
