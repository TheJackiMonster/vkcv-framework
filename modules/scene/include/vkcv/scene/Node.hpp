#pragma once

#include <vector>

#include "Mesh.hpp"

namespace vkcv::scene {
	
	class Node {
	private:
		std::vector<Mesh> m_models;
		std::vector<Node> m_nodes;
	
	public:
	};
	
}
