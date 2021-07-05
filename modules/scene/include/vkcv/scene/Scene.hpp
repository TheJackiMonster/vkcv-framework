#pragma once

#include <filesystem>

#include <vkcv/Core.hpp>
#include <vkcv/camera/Camera.hpp>
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
		
		std::vector<Material> m_materials;
		std::vector<Node> m_nodes;
		
		explicit Scene(Core* core);
		
	public:
		~Scene();
		
		Scene(const Scene& other);
		Scene(Scene&& other) noexcept;
		
		Scene& operator=(const Scene& other);
		Scene& operator=(Scene&& other) noexcept;
		
		Node& addNode();
		
		size_t getMaterialCount() const;
		
		[[nodiscard]]
		const material::Material& getMaterial(size_t index) const;
		
		void recordDrawcalls(CommandStreamHandle       		 &cmdStream,
							 const camera::Camera			 &camera,
							 const PassHandle                &pass,
							 const PipelineHandle            &pipeline,
							 const std::vector<ImageHandle>  &renderTargets);
		
		static Scene create(Core& core);
		
		static Scene load(Core& core, const std::filesystem::path &path);
		
	};
	
}