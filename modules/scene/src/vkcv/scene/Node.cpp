
#include "vkcv/scene/Node.hpp"
#include "vkcv/scene/Scene.hpp"
#include "vkcv/scene/Frustum.hpp"

#include <algorithm>

namespace vkcv::scene {
	
	Node::Node(Scene& scene) :
	m_scene(scene),
	m_meshes(),
	m_nodes(),
	m_bounds() {}
	
	Node::~Node() {
		m_nodes.clear();
		m_meshes.clear();
	}
	
	Node &Node::operator=(const Node &other) {
		if (&other == this) {
			return *this;
		}
		
		m_meshes.resize(other.m_meshes.size(), Mesh(m_scene));
		
		for (size_t i = 0; i < m_meshes.size(); i++) {
			m_meshes[i] = other.m_meshes[i];
		}
		
		m_nodes.resize(other.m_nodes.size(), Node(m_scene));
		
		for (size_t i = 0; i < m_nodes.size(); i++) {
			m_nodes[i] = other.m_nodes[i];
		}
		
		m_bounds = other.m_bounds;
		
		return *this;
	}
	
	Node &Node::operator=(Node &&other) noexcept {
		m_meshes.resize(other.m_meshes.size(), Mesh(m_scene));
		
		for (size_t i = 0; i < m_meshes.size(); i++) {
			m_meshes[i] = std::move(other.m_meshes[i]);
		}
		
		m_nodes.resize(other.m_nodes.size(), Node(m_scene));
		
		for (size_t i = 0; i < m_nodes.size(); i++) {
			m_nodes[i] = std::move(other.m_nodes[i]);
		}
		
		m_bounds = other.m_bounds;
		
		return *this;
	}
	
	size_t Node::getNodeCount() const {
		size_t count = 1;
		
		for (const auto& node : m_nodes) {
			count += node.getNodeCount();
		}
		
		return count;
	}
	
	size_t Node::getMeshCount() const {
		size_t count = 0;
		
		for (auto& mesh : m_meshes) {
			count++;
		}
		
		for (const auto& node : m_nodes) {
			count += node.getMeshCount();
		}
		
		return count;
	}
	
	size_t Node::getMeshPartCount() const {
		size_t count = 0;
		
		for (auto& mesh : m_meshes) {
			count += mesh.m_parts.size();
		}
		
		for (const auto& node : m_nodes) {
			count += node.getMeshPartCount();
		}
		
		return count;
	}
	
	void Node::addMesh(const Mesh& mesh) {
		if (m_meshes.empty()) {
			m_bounds = mesh.getBounds();
		} else {
			m_bounds.extend(mesh.getBounds().getMin());
			m_bounds.extend(mesh.getBounds().getMax());
		}
		
		m_meshes.push_back(mesh);
	}
	
	void Node::loadMesh(const asset::Scene &asset_scene,
						const asset::Mesh &asset_mesh,
						const std::vector<asset::PrimitiveType>& types) {
		Mesh mesh (m_scene);
		mesh.load(asset_scene, asset_mesh, types);
		addMesh(mesh);
	}
	
	size_t Node::addNode() {
		const Node node (m_scene);
		const size_t index = m_nodes.size();
		m_nodes.push_back(node);
		return index;
	}
	
	Node& Node::getNode(size_t index) {
		return m_nodes[index];
	}
	
	const Node& Node::getNode(size_t index) const {
		return m_nodes[index];
	}
	
	void Node::recordDrawcalls(const glm::mat4& viewProjection,
							   PushConstants& pushConstants,
							   std::vector<InstanceDrawcall>& drawcalls,
							   const RecordMeshDrawcallFunction& record) {
		if (!checkFrustum(viewProjection, m_bounds)) {
			return;
		}
		
		for (auto& mesh : m_meshes) {
			mesh.recordDrawcalls(viewProjection, pushConstants, drawcalls, record);
		}
		
		for (auto& node : m_nodes) {
			node.recordDrawcalls(viewProjection, pushConstants, drawcalls, record);
		}
	}
	
	void Node::appendAccelerationStructures(Core& core,
			std::vector<AccelerationStructureHandle> &accelerationStructures,
			const ProcessGeometryFunction &process) const {
		for (auto& mesh : m_meshes) {
			mesh.appendAccelerationStructures(core, accelerationStructures, process);
		}
		
		for (auto& node : m_nodes) {
			node.appendAccelerationStructures(core, accelerationStructures, process);
		}
	}
	
	void Node::splitMeshesToSubNodes(size_t maxMeshesPerNode) {
		if (m_meshes.size() <= maxMeshesPerNode) {
			return;
		}
		
		const auto split = m_bounds.getCenter();
		int axis = 0;
		
		const auto size = m_bounds.getSize();
		
		if (size[1] > size[0]) {
			if (size[2] > size[1]) {
				axis = 2;
			} else {
				axis = 1;
			}
		} else
		if (size[2] > size[0]) {
			axis = 2;
		}
		
		std::vector<size_t> left_meshes;
		std::vector<size_t> right_meshes;
		
		for (size_t i = 0; i < m_meshes.size(); i++) {
			const auto& bounds = m_meshes[i].getBounds();
			
			if (bounds.getMax()[axis] <= split[axis]) {
				left_meshes.push_back(i);
			} else
			if (bounds.getMin()[axis] >= split[axis]) {
				right_meshes.push_back(i);
			}
		}
		
		if ((left_meshes.empty()) || (right_meshes.empty())) {
			return;
		}
		
		const size_t left = addNode();
		const size_t right = addNode();
		
		for (size_t i : left_meshes) {
			getNode(left).addMesh(m_meshes[i]);
		}
		
		for (size_t i : right_meshes) {
			getNode(right).addMesh(m_meshes[i]);
			left_meshes.push_back(i);
		}
		
		std::sort(left_meshes.begin(), left_meshes.end(), std::greater());
		
		for (size_t i : left_meshes) {
			m_meshes.erase(m_meshes.begin() + static_cast<long>(i));
		}
		
		getNode(left).splitMeshesToSubNodes(maxMeshesPerNode);
		getNode(right).splitMeshesToSubNodes(maxMeshesPerNode);
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
