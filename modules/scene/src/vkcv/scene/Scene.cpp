
#include "vkcv/scene/Scene.hpp"

#include <vkcv/Logger.hpp>
#include <vkcv/asset/asset_loader.hpp>

namespace vkcv::scene {
	
	Scene::Scene(Core* core) :
	m_core(core),
	m_materials(),
	m_nodes() {}
	
	Scene::~Scene() {
		m_nodes.clear();
		m_materials.clear();
	}
	
	Scene::Scene(const Scene &other) :
	m_core(other.m_core),
	m_materials(other.m_materials),
	m_nodes(other.m_nodes) {}
	
	Scene::Scene(Scene &&other) noexcept :
	m_core(other.m_core),
	m_materials(other.m_materials),
	m_nodes(other.m_nodes) {}
	
	Scene &Scene::operator=(const Scene &other) {
		if (&other == this) {
			return *this;
		}
		
		m_core = other.m_core;
		m_materials = std::vector<Material>(other.m_materials);
		m_nodes = std::vector<Node>(other.m_nodes);
		
		return *this;
	}
	
	Scene &Scene::operator=(Scene &&other) noexcept {
		m_core = other.m_core;
		m_materials = std::move(other.m_materials);
		m_nodes = std::move(other.m_nodes);
		
		return *this;
	}
	
	Node& Scene::addNode() {
		Node node (this);
		m_nodes.push_back(node);
		return m_nodes.back();
	}
	
	size_t Scene::getMaterialCount() const {
		return m_materials.size();
	}
	
	const material::Material & Scene::getMaterial(size_t index) const {
		static material::Material noMaterial;
		
		if (index >= m_materials.size()) {
			return noMaterial;
		}
		
		return m_materials[index].m_data;
	}
	
	void Scene::recordDrawcalls(CommandStreamHandle       		&cmdStream,
								const camera::Camera			&camera,
								const PassHandle                &pass,
								const PipelineHandle            &pipeline,
								const std::vector<ImageHandle>  &renderTargets) {
		std::vector<glm::mat4> matrices;
		std::vector<DrawcallInfo> drawcalls;
		
		const glm::mat4 viewProjection = camera.getMVP();
		
		for (auto& node : m_nodes) {
			node.recordDrawcalls(viewProjection, matrices, drawcalls);
		}
		
		PushConstantData pushConstantData (matrices.data(), sizeof(glm::mat4));
		
		m_core->recordDrawcallsToCmdStream(
				cmdStream,
				pass,
				pipeline,
				pushConstantData,
				drawcalls,
				renderTargets
		);
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
