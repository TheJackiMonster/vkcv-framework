#pragma once

#include <filesystem>

#include <vkcv/Core.hpp>
#include <vkcv/material/Material.hpp>

#include "Node.hpp"

namespace vkcv::scene {
	
	class Scene {
		friend class MeshPart;
		
	private:
		struct Material {
			size_t m_usages;
			material::Material m_data;
		};
		
		Core* m_core;
		
		std::vector<Node> m_nodes;
		std::vector<Material> m_materials;
		
		explicit Scene(Core* core);
		
	public:
		~Scene() = default;
		
		Scene(const Scene& other) = default;
		Scene(Scene&& other) = default;
		
		Scene& operator=(const Scene& other) = default;
		Scene& operator=(Scene&& other) = default;
		
		Node& addNode();
		
		static Scene create(Core& core);
		
		static Scene load(Core& core, const std::filesystem::path &path);
		
	};
	
}