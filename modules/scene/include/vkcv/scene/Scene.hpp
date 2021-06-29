#pragma once

#include <filesystem>

#include <vkcv/material/Material.hpp>

#include "Node.hpp"

namespace vkcv::scene {
	
	class Scene {
	private:
		struct Material {
			size_t m_usages;
			material::Material m_data;
		};
		
		std::vector<Node> m_nodes;
		std::vector<Material> m_materials;
		
		Scene() = default;
		
	public:
		~Scene() = default;
		
		Scene(const Scene& other) = default;
		Scene(Scene&& other) = default;
		
		Scene& operator=(const Scene& other) = default;
		Scene& operator=(Scene&& other) = default;
		
		static Scene create();
		
		static Scene load(const std::filesystem::path &path);
		
	};
	
}