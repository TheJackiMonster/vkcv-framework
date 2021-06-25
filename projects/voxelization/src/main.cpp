#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/Logger.hpp>
#include "Voxelization.hpp"
#include <glm/glm.hpp>
#include "vkcv/gui/GUI.hpp"
#include "ShadowMapping.hpp"

int main(int argc, const char** argv) {
	const char* applicationName = "Voxelization";

	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;
	const vkcv::Multisampling   msaa        = vkcv::Multisampling::MSAA4X;
	const bool                  usingMsaa   = msaa != vkcv::Multisampling::None;
	
	vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		true
	);

    vkcv::camera::CameraManager cameraManager(window);
    uint32_t camIndex = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
    uint32_t camIndex2 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
    
    cameraManager.getCamera(camIndex).setPosition(glm::vec3(0.f, 0.f, 3.f));
    cameraManager.getCamera(camIndex).setNearFar(0.1f, 30.0f);
	cameraManager.getCamera(camIndex).setYaw(180.0f);
	
	cameraManager.getCamera(camIndex2).setNearFar(0.1f, 30.0f);

	window.initEvents();

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		{},
		{ "VK_KHR_swapchain" }
	);

	vkcv::asset::Scene mesh;

	const char* path = argc > 1 ? argv[1] : "resources/Sponza/Sponza.gltf";
	vkcv::asset::Scene scene;
	int result = vkcv::asset::loadScene(path, scene);

	if (result == 1) {
		std::cout << "Scene loading successful!" << std::endl;
	}
	else {
		std::cout << "Scene loading failed: " << result << std::endl;
		return 1;
	}

	// build index and vertex buffers
	assert(!scene.vertexGroups.empty());
	std::vector<std::vector<uint8_t>> vBuffers;
	std::vector<std::vector<uint8_t>> iBuffers;

	std::vector<vkcv::VertexBufferBinding> vBufferBindings;
	std::vector<std::vector<vkcv::VertexBufferBinding>> vertexBufferBindings;
	std::vector<vkcv::asset::VertexAttribute> vAttributes;

	for (int i = 0; i < scene.vertexGroups.size(); i++) {

		vBuffers.push_back(scene.vertexGroups[i].vertexBuffer.data);
		iBuffers.push_back(scene.vertexGroups[i].indexBuffer.data);

		auto& attributes = scene.vertexGroups[i].vertexBuffer.attributes;

		std::sort(attributes.begin(), attributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
			return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
		});
	}

	std::vector<vkcv::Buffer<uint8_t>> vertexBuffers;
	for (const vkcv::asset::VertexGroup& group : scene.vertexGroups) {
		vertexBuffers.push_back(core.createBuffer<uint8_t>(
			vkcv::BufferType::VERTEX,
			group.vertexBuffer.data.size()));
		vertexBuffers.back().fill(group.vertexBuffer.data);
	}

	std::vector<vkcv::Buffer<uint8_t>> indexBuffers;
	for (const auto& dataBuffer : iBuffers) {
		indexBuffers.push_back(core.createBuffer<uint8_t>(
			vkcv::BufferType::INDEX,
			dataBuffer.size()));
		indexBuffers.back().fill(dataBuffer);
	}

	int vertexBufferIndex = 0;
	for (const auto& vertexGroup : scene.vertexGroups) {
		for (const auto& attribute : vertexGroup.vertexBuffer.attributes) {
			vAttributes.push_back(attribute);
			vBufferBindings.push_back(vkcv::VertexBufferBinding(attribute.offset, vertexBuffers[vertexBufferIndex].getVulkanHandle()));
		}
		vertexBufferBindings.push_back(vBufferBindings);
		vBufferBindings.clear();
		vertexBufferIndex++;
	}

	const vk::Format colorBufferFormat = vk::Format::eB10G11R11UfloatPack32;
	const vkcv::AttachmentDescription color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		colorBufferFormat
	);
	
	const vk::Format depthBufferFormat = vk::Format::eD32Sfloat;
	const vkcv::AttachmentDescription depth_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		depthBufferFormat
	);
	
	// forward shading config
	vkcv::PassConfig forwardPassDefinition({ color_attachment, depth_attachment }, msaa);
	vkcv::PassHandle forwardPass = core.createPass(forwardPassDefinition);

	vkcv::shader::GLSLCompiler compiler;

	vkcv::ShaderProgram forwardProgram;
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"), 
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		forwardProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		forwardProgram.addShader(shaderStage, path);
	});

	const std::vector<vkcv::VertexAttachment> vertexAttachments = forwardProgram.getVertexAttachments();

	std::vector<vkcv::VertexBinding> vertexBindings;
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		vertexBindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
	}
	const vkcv::VertexLayout vertexLayout (vertexBindings);

	vkcv::DescriptorSetHandle forwardShadingDescriptorSet = 
		core.createDescriptorSet({ forwardProgram.getReflectedDescriptors()[0] });

	// depth prepass config
	vkcv::ShaderProgram depthPrepassShader;
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/depthPrepass.vert"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		depthPrepassShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/depthPrepass.frag"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		depthPrepassShader.addShader(shaderStage, path);
	});

	const std::vector<vkcv::VertexAttachment> prepassVertexAttachments = depthPrepassShader.getVertexAttachments();

	std::vector<vkcv::VertexBinding> prepassVertexBindings;
	for (size_t i = 0; i < prepassVertexAttachments.size(); i++) {
		prepassVertexBindings.push_back(vkcv::VertexBinding(i, { prepassVertexAttachments[i] }));
	}
	const vkcv::VertexLayout prepassVertexLayout(prepassVertexBindings);

	const vkcv::AttachmentDescription prepassAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		depthBufferFormat);

	vkcv::PassConfig prepassPassDefinition({ prepassAttachment }, msaa);
	vkcv::PassHandle prepassPass = core.createPass(prepassPassDefinition);

	// create descriptor sets
	vkcv::SamplerHandle colorSampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::REPEAT
	);

	std::vector<vkcv::DescriptorSetHandle> materialDescriptorSets;
	std::vector<vkcv::Image> sceneImages;

	for (const auto& material : scene.materials) {
		int albedoIndex     = material.baseColor;
		int normalIndex     = material.normal;
		int specularIndex   = material.metalRough;

		if (albedoIndex < 0) {
			vkcv_log(vkcv::LogLevel::WARNING, "Material lacks albedo");
			albedoIndex = 0;
		}
		if (normalIndex < 0) {
			vkcv_log(vkcv::LogLevel::WARNING, "Material lacks normal");
			normalIndex = 0;
		}
		if (specularIndex < 0) {
			vkcv_log(vkcv::LogLevel::WARNING, "Material lacks specular");
			specularIndex = 0;
		}

		materialDescriptorSets.push_back(core.createDescriptorSet(forwardProgram.getReflectedDescriptors()[1]));

		vkcv::asset::Texture& albedoTexture     = scene.textures[albedoIndex];
		vkcv::asset::Texture& normalTexture     = scene.textures[normalIndex];
		vkcv::asset::Texture& specularTexture   = scene.textures[specularIndex];

		// albedo texture
		sceneImages.push_back(core.createImage(vk::Format::eR8G8B8A8Srgb, albedoTexture.w, albedoTexture.h, 1, true));
		sceneImages.back().fill(albedoTexture.data.data());
		sceneImages.back().generateMipChainImmediate();
		sceneImages.back().switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		const vkcv::ImageHandle albedoHandle = sceneImages.back().getHandle();

		// normal texture
		sceneImages.push_back(core.createImage(vk::Format::eR8G8B8A8Unorm, normalTexture.w, normalTexture.h, 1, true));
		sceneImages.back().fill(normalTexture.data.data());
		sceneImages.back().generateMipChainImmediate();
		sceneImages.back().switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		const vkcv::ImageHandle normalHandle = sceneImages.back().getHandle();

		// specular texture
		sceneImages.push_back(core.createImage(vk::Format::eR8G8B8A8Unorm, specularTexture.w, specularTexture.h, 1, true));
		sceneImages.back().fill(specularTexture.data.data());
		sceneImages.back().generateMipChainImmediate();
		sceneImages.back().switchLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		const vkcv::ImageHandle specularHandle = sceneImages.back().getHandle();

		vkcv::DescriptorWrites setWrites;
		setWrites.sampledImageWrites = {
			vkcv::SampledImageDescriptorWrite(0, albedoHandle),
			vkcv::SampledImageDescriptorWrite(2, normalHandle),
			vkcv::SampledImageDescriptorWrite(3, specularHandle)
		};
		setWrites.samplerWrites = {
			vkcv::SamplerDescriptorWrite(1, colorSampler),
		};
		core.writeDescriptorSet(materialDescriptorSets.back(), setWrites);
	}

	std::vector<vkcv::DescriptorSetHandle> perMeshDescriptorSets;
	for (const auto& vertexGroup : scene.vertexGroups) {
		perMeshDescriptorSets.push_back(materialDescriptorSets[vertexGroup.materialIndex]);
	}

	// prepass pipeline
	vkcv::DescriptorSetHandle prepassDescriptorSet = core.createDescriptorSet(std::vector<vkcv::DescriptorBinding>());

	vkcv::PipelineConfig prepassPipelineConfig{
		depthPrepassShader,
		windowWidth,
		windowHeight,
		prepassPass,
		vertexLayout,
		{ 
			core.getDescriptorSet(prepassDescriptorSet).layout,
			core.getDescriptorSet(perMeshDescriptorSets[0]).layout },
		true };
	prepassPipelineConfig.m_culling         = vkcv::CullMode::Back;
	prepassPipelineConfig.m_multisampling   = msaa;
	prepassPipelineConfig.m_depthTest       = vkcv::DepthTest::LessEqual;
	prepassPipelineConfig.m_alphaToCoverage = true;

	vkcv::PipelineHandle prepassPipeline = core.createGraphicsPipeline(prepassPipelineConfig);

	// forward pipeline
	vkcv::PipelineConfig forwardPipelineConfig {
		forwardProgram,
		windowWidth,
		windowHeight,
		forwardPass,
		vertexLayout,
		{	
			core.getDescriptorSet(forwardShadingDescriptorSet).layout, 
			core.getDescriptorSet(perMeshDescriptorSets[0]).layout },
		true
	};
    forwardPipelineConfig.m_culling         = vkcv::CullMode::Back;
	forwardPipelineConfig.m_multisampling   = msaa;
	forwardPipelineConfig.m_depthTest       = vkcv::DepthTest::Equal;
	forwardPipelineConfig.m_depthWrite      = false;
	
	vkcv::PipelineHandle forwardPipeline = core.createGraphicsPipeline(forwardPipelineConfig);
	
	if (!forwardPipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	// sky
	struct SkySettings {
		glm::vec3   color;
		float       strength;
	};
	SkySettings skySettings;
	skySettings.color       = glm::vec3(0.15, 0.65, 1);
	skySettings.strength    = 5;

	const vkcv::AttachmentDescription skyColorAttachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		colorBufferFormat);

	const vkcv::AttachmentDescription skyDepthAttachments(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		depthBufferFormat);

	vkcv::PassConfig skyPassConfig({ skyColorAttachment, skyDepthAttachments }, msaa);
	vkcv::PassHandle skyPass = core.createPass(skyPassConfig);

	vkcv::ShaderProgram skyShader;
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/sky.vert"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		skyShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/sky.frag"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		skyShader.addShader(shaderStage, path);
	});

	vkcv::PipelineConfig skyPipeConfig;
	skyPipeConfig.m_ShaderProgram       = skyShader;
	skyPipeConfig.m_Width               = windowWidth;
	skyPipeConfig.m_Height              = windowHeight;
	skyPipeConfig.m_PassHandle          = skyPass;
	skyPipeConfig.m_VertexLayout        = vkcv::VertexLayout();
	skyPipeConfig.m_DescriptorLayouts   = {};
	skyPipeConfig.m_UseDynamicViewport  = true;
	skyPipeConfig.m_multisampling       = msaa;
	skyPipeConfig.m_depthWrite          = false;

	vkcv::PipelineHandle skyPipe = core.createGraphicsPipeline(skyPipeConfig);

	// render targets
	vkcv::ImageHandle depthBuffer           = core.createImage(depthBufferFormat, windowWidth, windowHeight, 1, false, false, false, msaa).getHandle();

    const bool colorBufferRequiresStorage   = !usingMsaa;
	vkcv::ImageHandle colorBuffer           = core.createImage(colorBufferFormat, windowWidth, windowHeight, 1, false, colorBufferRequiresStorage, true, msaa).getHandle();

	vkcv::ImageHandle resolvedColorBuffer;
	if (usingMsaa) {
		resolvedColorBuffer = core.createImage(colorBufferFormat, windowWidth, windowHeight, 1, false, true, true).getHandle();
	}
	else {
		resolvedColorBuffer = colorBuffer;
	}

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	bool renderVoxelVis = false;
	window.e_key.add([&renderVoxelVis](int key ,int scancode, int action, int mods) {
		if (key == GLFW_KEY_V && action == GLFW_PRESS) {
			renderVoxelVis = !renderVoxelVis;
		}
	});

	// tonemapping compute shader
	vkcv::ShaderProgram tonemappingProgram;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "resources/shaders/tonemapping.comp", 
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		tonemappingProgram.addShader(shaderStage, path);
	});
	vkcv::DescriptorSetHandle tonemappingDescriptorSet = core.createDescriptorSet(
		tonemappingProgram.getReflectedDescriptors()[0]);
	vkcv::PipelineHandle tonemappingPipeline = core.createComputePipeline(
		tonemappingProgram,
		{ core.getDescriptorSet(tonemappingDescriptorSet).layout });

	// resolve compute shader
	vkcv::ShaderProgram resolveProgram;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "resources/shaders/msaa4XResolve.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		resolveProgram.addShader(shaderStage, path);
	});
	vkcv::DescriptorSetHandle resolveDescriptorSet = core.createDescriptorSet(
		resolveProgram.getReflectedDescriptors()[0]);
	vkcv::PipelineHandle resolvePipeline = core.createComputePipeline(
		resolveProgram,
		{ core.getDescriptorSet(resolveDescriptorSet).layout });

	vkcv::SamplerHandle resolveSampler = core.createSampler(
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerMipmapMode::NEAREST,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE);

	// model matrices per mesh
	std::vector<glm::mat4> modelMatrices;
	modelMatrices.resize(scene.vertexGroups.size(), glm::mat4(1.f));
	for (const auto& mesh : scene.meshes) {
		const glm::mat4 m = *reinterpret_cast<const glm::mat4*>(&mesh.modelMatrix[0]);
		for (const auto& vertexGroupIndex : mesh.vertexGroups) {
			modelMatrices[vertexGroupIndex] = m;
		}
	}

	// prepare meshes
	std::vector<vkcv::Mesh> meshes;
	for (int i = 0; i < scene.vertexGroups.size(); i++) {
		vkcv::Mesh mesh(vertexBufferBindings[i], indexBuffers[i].getVulkanHandle(), scene.vertexGroups[i].numIndices);
		meshes.push_back(mesh);
	}

	std::vector<vkcv::DrawcallInfo> drawcalls;
	std::vector<vkcv::DrawcallInfo> prepassDrawcalls;
	for (int i = 0; i < meshes.size(); i++) {

		drawcalls.push_back(vkcv::DrawcallInfo(meshes[i], { 
			vkcv::DescriptorSetUsage(0, core.getDescriptorSet(forwardShadingDescriptorSet).vulkanHandle),
			vkcv::DescriptorSetUsage(1, core.getDescriptorSet(perMeshDescriptorSets[i]).vulkanHandle) }));
		prepassDrawcalls.push_back(vkcv::DrawcallInfo(meshes[i], {
			vkcv::DescriptorSetUsage(0, core.getDescriptorSet(prepassDescriptorSet).vulkanHandle),
			vkcv::DescriptorSetUsage(1, core.getDescriptorSet(perMeshDescriptorSets[i]).vulkanHandle) }));
	}

	vkcv::SamplerHandle voxelSampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE);

	ShadowMapping shadowMapping(&core, vertexLayout);

	Voxelization::Dependencies voxelDependencies;
	voxelDependencies.colorBufferFormat = colorBufferFormat;
	voxelDependencies.depthBufferFormat = depthBufferFormat;
	voxelDependencies.vertexLayout = vertexLayout;
	Voxelization voxelization(
		&core,
		voxelDependencies,
		shadowMapping.getLightInfoBuffer(),
		shadowMapping.getShadowMap(),
		shadowMapping.getShadowSampler(),
		voxelSampler,
		msaa);

	vkcv::Buffer<glm::vec3> cameraPosBuffer = core.createBuffer<glm::vec3>(vkcv::BufferType::UNIFORM, 1);

	struct VolumetricSettings {
		glm::vec3   scatteringCoefficient;
		float       ambientLight;
		glm::vec3   absorptionCoefficient;
	};
	vkcv::Buffer<VolumetricSettings> volumetricSettingsBuffer
		= core.createBuffer<VolumetricSettings>(vkcv::BufferType::UNIFORM ,1);

	// write forward pass descriptor set
	vkcv::DescriptorWrites forwardDescriptorWrites;
	forwardDescriptorWrites.uniformBufferWrites = {
		vkcv::UniformBufferDescriptorWrite(0, shadowMapping.getLightInfoBuffer()),
		vkcv::UniformBufferDescriptorWrite(3, cameraPosBuffer.getHandle()),
		vkcv::UniformBufferDescriptorWrite(6, voxelization.getVoxelInfoBufferHandle()),
		vkcv::UniformBufferDescriptorWrite(7, volumetricSettingsBuffer.getHandle())};
	forwardDescriptorWrites.sampledImageWrites = {
		vkcv::SampledImageDescriptorWrite(1, shadowMapping.getShadowMap()),
		vkcv::SampledImageDescriptorWrite(4, voxelization.getVoxelImageHandle()) };
	forwardDescriptorWrites.samplerWrites = { 
		vkcv::SamplerDescriptorWrite(2, shadowMapping.getShadowSampler()),
		vkcv::SamplerDescriptorWrite(5, voxelSampler) };
	core.writeDescriptorSet(forwardShadingDescriptorSet, forwardDescriptorWrites);

	vkcv::gui::GUI gui(core, window);

	glm::vec2   lightAnglesDegree               = glm::vec2(90.f, 0.f);
	glm::vec3   lightColor                      = glm::vec3(1);
	float       lightStrength                   = 25.f;
	float       maxShadowDistance               = 30.f;

	int     voxelVisualisationMip   = 0;
	float   voxelizationExtent      = 30.f;

	bool msaaCustomResolve = true;

	glm::vec3   scatteringColor     = glm::vec3(1);
	float       scatteringDensity   = 0.001;
	glm::vec3   absorptionColor     = glm::vec3(1);
	float       absorptionDensity   = 0.001;
	float       volumetricAmbient   = 0.2;

	auto start = std::chrono::system_clock::now();
	const auto appStartTime = start;
	while (window.isWindowOpen()) {
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}

		if ((swapchainWidth != windowWidth) || ((swapchainHeight != windowHeight))) {
			depthBuffer         = core.createImage(depthBufferFormat, swapchainWidth, swapchainHeight, 1, false, false, false, msaa).getHandle();
			colorBuffer         = core.createImage(colorBufferFormat, swapchainWidth, swapchainHeight, 1, false, colorBufferRequiresStorage, true, msaa).getHandle();

			if (usingMsaa) {
				resolvedColorBuffer = core.createImage(colorBufferFormat, swapchainWidth, swapchainHeight, 1, false, true, true).getHandle();
			}
			else {
				resolvedColorBuffer = colorBuffer;
			}

			windowWidth = swapchainWidth;
			windowHeight = swapchainHeight;
		}

		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		// update descriptor sets which use swapchain image
		vkcv::DescriptorWrites tonemappingDescriptorWrites;
		tonemappingDescriptorWrites.storageImageWrites = {
			vkcv::StorageImageDescriptorWrite(0, resolvedColorBuffer),
			vkcv::StorageImageDescriptorWrite(1, swapchainInput) };
		core.writeDescriptorSet(tonemappingDescriptorSet, tonemappingDescriptorWrites);

		// update resolve descriptor, color images could be changed
		vkcv::DescriptorWrites resolveDescriptorWrites;
		resolveDescriptorWrites.sampledImageWrites  = { vkcv::SampledImageDescriptorWrite(0, colorBuffer) };
		resolveDescriptorWrites.samplerWrites       = { vkcv::SamplerDescriptorWrite(1, resolveSampler) };
		resolveDescriptorWrites.storageImageWrites  = { vkcv::StorageImageDescriptorWrite(2, resolvedColorBuffer) };
		core.writeDescriptorSet(resolveDescriptorSet, resolveDescriptorWrites);

		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
		cameraPosBuffer.fill({ cameraManager.getActiveCamera().getPosition() });

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		voxelization.updateVoxelOffset(cameraManager.getActiveCamera());

		// shadow map
		glm::vec2 lightAngleRadian = glm::radians(lightAnglesDegree);
		shadowMapping.recordShadowMapRendering(
			cmdStream,
			lightAngleRadian,
			lightColor,
			lightStrength,
			maxShadowDistance,
			meshes,
			modelMatrices,
			cameraManager.getActiveCamera(),
			voxelization.getVoxelOffset(),
			voxelization.getVoxelExtent());

		// voxelization
		voxelization.setVoxelExtent(voxelizationExtent);
		voxelization.voxelizeMeshes(
			cmdStream,
			meshes, 
			modelMatrices,
			perMeshDescriptorSets);

		// depth prepass
		const glm::mat4 viewProjectionCamera = cameraManager.getActiveCamera().getMVP();

		std::vector<glm::mat4> prepassMatrices;
		for (const auto& m : modelMatrices) {
			prepassMatrices.push_back(viewProjectionCamera * m);
		}

		const vkcv::PushConstantData            prepassPushConstantData((void*)prepassMatrices.data(), sizeof(glm::mat4));
		const std::vector<vkcv::ImageHandle>    prepassRenderTargets = { depthBuffer };

		core.recordDrawcallsToCmdStream(
			cmdStream,
			prepassPass,
			prepassPipeline,
			prepassPushConstantData,
			prepassDrawcalls,
			prepassRenderTargets);

		core.recordImageMemoryBarrier(cmdStream, depthBuffer);

		// main pass
		std::vector<std::array<glm::mat4, 2>> mainPassMatrices;
		for (const auto& m : modelMatrices) {
			mainPassMatrices.push_back({ viewProjectionCamera * m, m });
		}

		VolumetricSettings volumeSettings;
		volumeSettings.scatteringCoefficient    = scatteringColor * scatteringDensity;
		volumeSettings.absorptionCoefficient    = absorptionColor * absorptionDensity;
		volumeSettings.ambientLight             = volumetricAmbient;
		volumetricSettingsBuffer.fill({ volumeSettings });

		const vkcv::PushConstantData            pushConstantData((void*)mainPassMatrices.data(), 2 * sizeof(glm::mat4));
		const std::vector<vkcv::ImageHandle>    renderTargets = { colorBuffer, depthBuffer };

		core.recordDrawcallsToCmdStream(
			cmdStream,
			forwardPass,
			forwardPipeline,
			pushConstantData,
			drawcalls,
			renderTargets);

		if (renderVoxelVis) {
			voxelization.renderVoxelVisualisation(cmdStream, viewProjectionCamera, renderTargets, voxelVisualisationMip);
		}

		// sky
		core.recordDrawcallsToCmdStream(
			cmdStream,
			skyPass,
			skyPipe,
			vkcv::PushConstantData((void*)&skySettings, sizeof(skySettings)),
			{ vkcv::DrawcallInfo(vkcv::Mesh({}, nullptr, 3), {}) },
			renderTargets);

		const uint32_t fullscreenLocalGroupSize = 8;
		const uint32_t fulsscreenDispatchCount[3] = {
			static_cast<uint32_t>(glm::ceil(windowWidth  / static_cast<float>(fullscreenLocalGroupSize))),
			static_cast<uint32_t>(glm::ceil(windowHeight / static_cast<float>(fullscreenLocalGroupSize))),
			1
		};

		if (usingMsaa) {
			if (msaaCustomResolve) {

				core.prepareImageForSampling(cmdStream, colorBuffer);
				core.prepareImageForStorage(cmdStream, resolvedColorBuffer);

				assert(msaa == vkcv::Multisampling::MSAA4X);	// shaders is written for msaa 4x
				core.recordComputeDispatchToCmdStream(
					cmdStream,
					resolvePipeline,
					fulsscreenDispatchCount,
					{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(resolveDescriptorSet).vulkanHandle) },
					vkcv::PushConstantData(nullptr, 0));

				core.recordImageMemoryBarrier(cmdStream, resolvedColorBuffer);
			}
			else {
				core.resolveMSAAImage(cmdStream, colorBuffer, resolvedColorBuffer);
			}
		}

		core.prepareImageForStorage(cmdStream, swapchainInput);
		core.prepareImageForStorage(cmdStream, resolvedColorBuffer);

		auto timeSinceStart = std::chrono::duration_cast<std::chrono::microseconds>(end - appStartTime);
		float timeF         = static_cast<float>(timeSinceStart.count()) * 0.01;

		core.recordComputeDispatchToCmdStream(
			cmdStream, 
			tonemappingPipeline, 
			fulsscreenDispatchCount,
			{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(tonemappingDescriptorSet).vulkanHandle) },
			vkcv::PushConstantData(&timeF, sizeof(timeF)));

		// present and end
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		// draw UI
		gui.beginGUI();

		ImGui::Begin("Settings");

		ImGui::Checkbox("MSAA custom resolve", &msaaCustomResolve);

		ImGui::DragFloat2("Light angles",           &lightAnglesDegree.x);
		ImGui::ColorEdit3("Sun color",              &lightColor.x);
		ImGui::DragFloat("Sun strength",            &lightStrength);
		ImGui::DragFloat("Max shadow distance",     &maxShadowDistance);
		maxShadowDistance = std::max(maxShadowDistance, 1.f);

		ImGui::ColorEdit3("Sky color",      &skySettings.color.x);
		ImGui::DragFloat("Sky strength",    &skySettings.strength, 0.1);

		ImGui::Checkbox("Draw voxel visualisation", &renderVoxelVis);
		ImGui::SliderInt("Visualisation mip",       &voxelVisualisationMip, 0, 7);
		ImGui::DragFloat("Voxelization extent",     &voxelizationExtent, 1.f, 0.f);
		voxelizationExtent = std::max(voxelizationExtent, 1.f);
		voxelVisualisationMip = std::max(voxelVisualisationMip, 0);

		ImGui::ColorEdit3("Scattering color", &scatteringColor.x);
		ImGui::DragFloat("Scattering density", &scatteringDensity, 0.0001);
		ImGui::ColorEdit3("Absorption color", &absorptionColor.x);
		ImGui::DragFloat("Absorption density", &absorptionDensity, 0.0001);
		ImGui::DragFloat("Volumetric ambient", &volumetricAmbient, 0.002);

		if (ImGui::Button("Reload forward pass")) {

			vkcv::ShaderProgram newForwardProgram;
			compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"),
				[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
				newForwardProgram.addShader(shaderStage, path);
			});
			compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
				[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
				newForwardProgram.addShader(shaderStage, path);
			});
			forwardPipelineConfig.m_ShaderProgram = newForwardProgram;
			vkcv::PipelineHandle newPipeline = core.createGraphicsPipeline(forwardPipelineConfig);

			if (newPipeline) {
				forwardPipeline = newPipeline;
			}
		}
		if (ImGui::Button("Reload tonemapping")) {

			vkcv::ShaderProgram newProgram;
			compiler.compile(vkcv::ShaderStage::COMPUTE, std::filesystem::path("resources/shaders/tonemapping.comp"),
				[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
				newProgram.addShader(shaderStage, path);
			});
			vkcv::PipelineHandle newPipeline = core.createComputePipeline(
				newProgram, 
				{ core.getDescriptorSet(tonemappingDescriptorSet).layout });

			if (newPipeline) {
				tonemappingPipeline = newPipeline;
			}
		}

		ImGui::End();

		gui.endGUI();

		core.endFrame();
	}
	
	return 0;
}
