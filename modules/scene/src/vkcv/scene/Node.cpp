
#include "vkcv/scene/Node.hpp"
#include "Frustum.hpp"

namespace vkcv::scene {
	
	Node::Node(Scene* scene) :
	m_scene(scene),
	m_meshes(),
	m_nodes(),
	m_bounds() {}
	
	Node::~Node() {
		m_nodes.clear();
		m_meshes.clear();
	}
	
	Node::Node(const Node &other) :
	m_scene(other.m_scene),
	m_meshes(other.m_meshes),
	m_nodes(other.m_nodes),
	m_bounds(other.m_bounds) {}
	
	Node::Node(Node &&other) noexcept :
	m_scene(other.m_scene),
	m_meshes(other.m_meshes),
	m_nodes(other.m_nodes),
	m_bounds(other.m_bounds) {}
	
	Node &Node::operator=(const Node &other) {
		if (&other == this) {
			return *this;
		}
		
		m_scene = other.m_scene;
		m_meshes = std::vector<Mesh>(other.m_meshes);
		m_nodes = std::vector<Node>(other.m_nodes);
		m_bounds = other.m_bounds;
		
		return *this;
	}
	
	Node &Node::operator=(Node &&other) noexcept {
		m_scene = other.m_scene;
		m_meshes = std::move(other.m_meshes);
		m_nodes = std::move(other.m_nodes);
		m_bounds = other.m_bounds;
		
		return *this;
	}
	
	void Node::loadMesh(const asset::Scene &asset_scene, const asset::Mesh &asset_mesh) {
		Mesh mesh (m_scene);
		mesh.load(asset_scene, asset_mesh);
		
		if (m_meshes.empty()) {
			m_bounds = mesh.getBounds();
		} else {
			m_bounds.extend(mesh.getBounds().getMin());
			m_bounds.extend(mesh.getBounds().getMax());
		}
		
		m_meshes.push_back(mesh);
	}
	
	Node& Node::addNode() {
		Node node (this->m_scene);
		m_nodes.push_back(node);
		return m_nodes.back();
	}
	
	void Node::recordDrawcalls(const glm::mat4& viewProjection,
							   std::vector<glm::mat4>& matrices,
							   std::vector<DrawcallInfo>& drawcalls) {
		if (!checkFrustum(viewProjection, m_bounds)) {
			return;
		}
		
		for (auto& mesh : m_meshes) {
			mesh.recordDrawcalls(viewProjection, matrices, drawcalls);
		}
		
		for (auto& node : m_nodes) {
			node.recordDrawcalls(viewProjection, matrices, drawcalls);
		}
	}
	
	size_t Node::getDrawcallCount() const {
		size_t count = 0;
		
		for (auto& mesh : m_meshes) {
			count += mesh.getDrawcallCount();
		}
		
		for (auto& node : m_nodes) {
			count += node.getDrawcallCount();
		}
		
		return count;
	}
	
	const Bounds& Node::getBounds() const {
		return m_bounds;
	}

}
