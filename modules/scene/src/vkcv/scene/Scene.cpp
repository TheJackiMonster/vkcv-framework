
#include "vkcv/scene/Scene.hpp"

#include <vkcv/Logger.hpp>
#include <vkcv/asset/asset_loader.hpp>

namespace vkcv::scene {
	
	Scene::Scene(Core* core) :
	m_core(core) {}
	
	Node& Scene::addNode() {
		Node node (this);
		m_nodes.push_back(node);
		return m_nodes.back();
	}
	
	Scene Scene::create(Core& core) {
		return Scene(&core);
	}
	
	Scene Scene::load(Core& core, const std::filesystem::path &path) {
		asset::Scene asset_scene;
		
		if (!asset::loadScene(path.string(), asset_scene)) {
			vkcv_log(LogLevel::WARNING, "Scene could not be loaded")
			return create(core);
		}
		
		Scene scene = create(core);
		
		for (const auto& material : asset_scene.materials) {
			scene.m_materials.push_back({
				0, material::Material()
			});
		}
		
		Node& node = scene.addNode();
		
		for (const auto& mesh : asset_scene.meshes) {
			node.loadMesh(asset_scene, mesh);
		}
		
		return scene;
	}
	
}
