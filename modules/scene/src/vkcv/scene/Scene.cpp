
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
	m_nodes() {
		m_nodes.resize(other.m_nodes.size(), Node(*this));
		
		for (size_t i = 0; i < m_nodes.size(); i++) {
			m_nodes[i] = other.m_nodes[i];
		}
	}
	
	Scene::Scene(Scene &&other) noexcept :
	m_core(other.m_core),
	m_materials(other.m_materials),
	m_nodes() {
		m_nodes.resize(other.m_nodes.size(), Node(*this));
		
		for (size_t i = 0; i < m_nodes.size(); i++) {
			m_nodes[i] = std::move(other.m_nodes[i]);
		}
	}
	
	Scene &Scene::operator=(const Scene &other) {
		if (&other == this) {
			return *this;
		}
		
		m_core = other.m_core;
		m_materials = std::vector<Material>(other.m_materials);
		
		m_nodes.resize(other.m_nodes.size(), Node(*this));
		
		for (size_t i = 0; i < m_nodes.size(); i++) {
			m_nodes[i] = other.m_nodes[i];
		}
		
		return *this;
	}
	
	Scene &Scene::operator=(Scene &&other) noexcept {
		m_core = other.m_core;
		m_materials = std::move(other.m_materials);
		
		m_nodes.resize(other.m_nodes.size(), Node(*this));
		
		for (size_t i = 0; i < m_nodes.size(); i++) {
			m_nodes[i] = std::move(other.m_nodes[i]);
		}
		
		return *this;
	}
	
	size_t Scene::addNode() {
		const Node node (*this);
		const size_t index = m_nodes.size();
		m_nodes.push_back(node);
		return index;
	}
	
	Node& Scene::getNode(size_t index) {
		return m_nodes[index];
	}
	
	const Node& Scene::getNode(size_t index) const {
		return m_nodes[index];
	}
	
	void Scene::increaseMaterialUsage(size_t index) {
		if (index < m_materials.size()) {
			m_materials[index].m_usages++;
		}
	}
	
	void Scene::decreaseMaterialUsage(size_t index) {
		if ((index < m_materials.size()) && (m_materials[index].m_usages > 0)) {
			m_materials[index].m_usages--;
		}
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
	
	void Scene::recordDrawcalls(CommandStreamHandle       		 &cmdStream,
								const camera::Camera			 &camera,
								const PassHandle                 &pass,
								const GraphicsPipelineHandle     &pipeline,
								size_t							 pushConstantsSizePerDrawcall,
								const RecordMeshDrawcallFunction &record,
								const std::vector<ImageHandle>   &renderTargets,
								const WindowHandle               &windowHandle) {
		m_core->recordBeginDebugLabel(cmdStream, "vkcv::scene::Scene", {
			0.0f, 1.0f, 0.0f, 1.0f
		});
		
		PushConstants pushConstants (pushConstantsSizePerDrawcall);
		std::vector<DrawcallInfo> drawcalls;
		size_t count = 0;
		
		const glm::mat4 viewProjection = camera.getMVP();
		
		for (auto& node : m_nodes) {
			count += node.getDrawcallCount();
			node.recordDrawcalls(viewProjection, pushConstants, drawcalls, record);
		}
		
		vkcv_log(LogLevel::RAW_INFO, "Frustum culling: %lu / %lu", drawcalls.size(), count);
		
		m_core->recordDrawcallsToCmdStream(
				cmdStream,
				pass,
				pipeline,
				pushConstants,
				drawcalls,
				renderTargets,
				windowHandle
		);
		
		m_core->recordEndDebugLabel(cmdStream);
	}
	
	Scene Scene::create(Core& core) {
		return Scene(&core);
	}
	
	static void loadImage(Core& core, const asset::Scene& asset_scene,
						  const asset::Texture& asset_texture,
						  const vk::Format& format,
						  ImageHandle& image, SamplerHandle& sampler) {
		asset::Sampler* asset_sampler = nullptr;
		
		if ((asset_texture.sampler >= 0) && (asset_texture.sampler < asset_scene.samplers.size())) {
			//asset_sampler = &(asset_scene.samplers[asset_texture.sampler]); // TODO
		}
		
		Image img = core.createImage(format, asset_texture.w, asset_texture.h);
		img.fill(asset_texture.data.data());
		image = img.getHandle();
		
		if (asset_sampler) {
			//sampler = core.createSampler(asset_sampler) // TODO
		} else {
			sampler = core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT
			);
		}
	}
	
	void Scene::loadMaterial(size_t index, const asset::Scene& scene,
							 const asset::Material& material) {
		if (index >= m_materials.size()) {
			return;
		}
		
		ImageHandle diffuseImg;
		SamplerHandle diffuseSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(*m_core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		ImageHandle normalImg;
		SamplerHandle normalSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(*m_core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		ImageHandle metalRoughImg;
		SamplerHandle metalRoughSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(*m_core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		ImageHandle occlusionImg;
		SamplerHandle occlusionSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(*m_core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		ImageHandle emissionImg;
		SamplerHandle emissionSmp;
		
		if ((material.baseColor >= 0) && (material.baseColor < scene.textures.size())) {
			loadImage(*m_core, scene, scene.textures[material.baseColor], vk::Format::eR8G8B8A8Srgb,
					  diffuseImg,diffuseSmp);
		}
		
		const float colorFactors [4] = {
				material.baseColorFactor.r,
				material.baseColorFactor.g,
				material.baseColorFactor.b,
				material.baseColorFactor.a
		};
		
		const float emissionFactors[4] = {
				material.emissiveFactor.r,
				material.emissiveFactor.g,
				material.emissiveFactor.b
		};
		
		m_materials[index].m_data = material::Material::createPBR(
				*m_core,
				diffuseImg, diffuseSmp,
				normalImg, normalSmp,
				metalRoughImg, metalRoughSmp,
				occlusionImg, occlusionSmp,
				emissionImg, emissionSmp,
				colorFactors,
				material.normalScale,
				material.metallicFactor,
				material.roughnessFactor,
				material.occlusionStrength,
				emissionFactors
		);
	}
	
	Scene Scene::load(Core& core, const std::filesystem::path &path) {
		asset::Scene asset_scene;
		
		if (!asset::loadScene(path.string(), asset_scene)) {
			vkcv_log(LogLevel::ERROR, "Scene could not be loaded (%s)", path.c_str());
			return create(core);
		}
		
		Scene scene = create(core);
		
		for (const auto& material : asset_scene.materials) {
			scene.m_materials.push_back({
				0, material::Material()
			});
		}
		
		const size_t root = scene.addNode();
		
		for (const auto& mesh : asset_scene.meshes) {
			scene.getNode(root).loadMesh(asset_scene, mesh);
		}
		
		auto mipStream = core.createCommandStream(vkcv::QueueType::Graphics);
		
		for (auto& material : scene.m_materials) {
			material.m_data.recordMipChainGeneration(mipStream, core.getDownsampler());
		}
		
		core.submitCommandStream(mipStream, false);
		
		scene.getNode(root).splitMeshesToSubNodes(128);
		return scene;
	}
	
}
