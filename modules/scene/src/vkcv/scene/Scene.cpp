
#include "vkcv/scene/Scene.hpp"

#include <vkcv/Logger.hpp>
#include <vkcv/asset/asset_loader.hpp>

namespace vkcv::scene {
	
	Scene Scene::create() {
		return Scene();
	}
	
	Scene Scene::load(const std::filesystem::path &path) {
		asset::Scene asset_scene;
		
		if (!asset::loadScene(path.string(), asset_scene)) {
			vkcv_log(LogLevel::WARNING, "Scene could not be loaded")
			return create();
		}
		
		Scene scene = create();
		
		for (const auto& material : asset_scene.materials) {
			scene.m_materials.push_back({
				0, material::Material()
			});
		}
		
		for (const auto& mesh : asset_scene.meshes) {
			//TODO
		}
		
		return scene;
	}
	
}
