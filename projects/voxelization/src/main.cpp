#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include "Voxelization.hpp"
#include "vkcv/gui/GUI.hpp"
#include "ShadowMapping.hpp"
#include <vkcv/upscaling/FSRUpscaling.hpp>
#include <vkcv/upscaling/BilinearUpscaling.hpp>
#include <vkcv/upscaling/NISUpscaling.hpp>
#include <vkcv/effects/BloomAndFlaresEffect.hpp>
#include <vkcv/algorithm/SinglePassDownsampler.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "Voxelization";

	const vkcv::Multisampling   msaa        = vkcv::Multisampling::MSAA4X;
	const bool                  usingMsaa   = msaa != vkcv::Multisampling::None;

	vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	
	features.requireExtensionFeature<vk::PhysicalDeviceDescriptorIndexingFeatures>(
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			[](vk::PhysicalDeviceDescriptorIndexingFeatures& features) {
				features.setDescriptorBindingPartiallyBound(true);
			}
	);
	
	features.tryExtensionFeature<vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures>(
		VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME,
		[](vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures& features) {
			features.setShaderSubgroupExtendedTypes(true);
		}
	);
	
	features.tryExtensionFeature<vk::PhysicalDevice16BitStorageFeatures>(
		VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
		[](vk::PhysicalDevice16BitStorageFeatures& features) {
			features.setStorageBuffer16BitAccess(true);
		}
	);

	features.tryExtensionFeature<vk::PhysicalDeviceShaderFloat16Int8Features>(
		VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
		[](vk::PhysicalDeviceShaderFloat16Int8Features& features) {
			features.setShaderFloat16(true);
		}
	);

	const uint32_t windowWidth = 1280;
	const uint32_t windowHeight = 720;

	vkcv::Core core = vkcv::Core::create(
			applicationName,
			VK_MAKE_VERSION(0, 0, 1),
			{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
			features
	);
	
	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, true);
	vkcv::Window& window = core.getWindow(windowHandle);

	bool     isFullscreen            = false;
	uint32_t windowedWidthBackup     = window.getWidth();
	uint32_t windowedHeightBackup    = window.getHeight();
	int      windowedPosXBackup;
	int      windowedPosYBackup;
    glfwGetWindowPos(window.getWindow(), &windowedPosXBackup, &windowedPosYBackup);

	window.e_key.add([&](int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
			if (isFullscreen) {
				glfwSetWindowMonitor(
					window.getWindow(),
					nullptr,
					windowedPosXBackup,
					windowedPosYBackup,
					windowedWidthBackup,
					windowedHeightBackup,
					GLFW_DONT_CARE);
			}
			else {
				windowedWidthBackup     = window.getWidth();
				windowedHeightBackup    = window.getHeight();

				glfwGetWindowPos(window.getWindow(), &windowedPosXBackup, &windowedPosYBackup);

				GLFWmonitor*        monitor     = glfwGetPrimaryMonitor();
				const GLFWvidmode*  videoMode   = glfwGetVideoMode(monitor);

				glfwSetWindowMonitor(
					window.getWindow(),
					glfwGetPrimaryMonitor(),
					0,
					0,
					videoMode->width,
					videoMode->height,
					videoMode->refreshRate);
			}
			isFullscreen = !isFullscreen;
		}
	});

	vkcv::camera::CameraManager cameraManager(window);
	uint32_t camIndex  = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex2 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

	cameraManager.getCamera(camIndex).setPosition(glm::vec3(0.f, 0.f, 3.f));
	cameraManager.getCamera(camIndex).setNearFar(0.1f, 30.0f);
	cameraManager.getCamera(camIndex).setYaw(180.0f);
	cameraManager.getCamera(camIndex).setFov(glm::radians(37.8));	// fov of a 35mm lens
	
	cameraManager.getCamera(camIndex2).setNearFar(0.1f, 30.0f);

	vkcv::asset::Scene mesh;

	const char* path = argc > 1 ? argv[1] : "assets/Sponza/Sponza.gltf";
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

	for (size_t i = 0; i < scene.vertexGroups.size(); i++) {

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
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("assets/shaders/shader.vert"), 
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		forwardProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("assets/shaders/shader.frag"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		forwardProgram.addShader(shaderStage, path);
	});

	const std::vector<vkcv::VertexAttachment> vertexAttachments = forwardProgram.getVertexAttachments();

	std::vector<vkcv::VertexBinding> vertexBindings;
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		vertexBindings.push_back(vkcv::createVertexBinding(i, { vertexAttachments[i] }));
	}
	const vkcv::VertexLayout vertexLayout { vertexBindings };

	vkcv::DescriptorSetLayoutHandle forwardShadingDescriptorSetLayout = core.createDescriptorSetLayout(forwardProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle forwardShadingDescriptorSet = core.createDescriptorSet(forwardShadingDescriptorSetLayout);

	// depth prepass config
	vkcv::ShaderProgram depthPrepassShader;
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("assets/shaders/depthPrepass.vert"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		depthPrepassShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("assets/shaders/depthPrepass.frag"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		depthPrepassShader.addShader(shaderStage, path);
	});

	const std::vector<vkcv::VertexAttachment> prepassVertexAttachments = depthPrepassShader.getVertexAttachments();

	std::vector<vkcv::VertexBinding> prepassVertexBindings;
	for (size_t i = 0; i < prepassVertexAttachments.size(); i++) {
		prepassVertexBindings.push_back(vkcv::createVertexBinding(i, { prepassVertexAttachments[i] }));
	}
	const vkcv::VertexLayout prepassVertexLayout { prepassVertexBindings };

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

	std::vector<vkcv::DescriptorSetLayoutHandle> materialDescriptorSetLayouts;
	std::vector<vkcv::DescriptorSetHandle> materialDescriptorSets;
	std::vector<vkcv::Image> sceneImages;
	
	vkcv::algorithm::SinglePassDownsampler spdDownsampler (core, colorSampler);
	
	auto mipStream = core.createCommandStream(vkcv::QueueType::Graphics);

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

		materialDescriptorSetLayouts.push_back(core.createDescriptorSetLayout(forwardProgram.getReflectedDescriptors().at(1)));
		materialDescriptorSets.push_back(core.createDescriptorSet(materialDescriptorSetLayouts.back()));

		vkcv::asset::Texture& albedoTexture     = scene.textures[albedoIndex];
		vkcv::asset::Texture& normalTexture     = scene.textures[normalIndex];
		vkcv::asset::Texture& specularTexture   = scene.textures[specularIndex];

		// albedo texture
		sceneImages.push_back(core.createImage(vk::Format::eR8G8B8A8Srgb, albedoTexture.w, albedoTexture.h, 1, true));
		sceneImages.back().fill(albedoTexture.data.data());
		sceneImages.back().recordMipChainGeneration(mipStream, spdDownsampler);
		const vkcv::ImageHandle albedoHandle = sceneImages.back().getHandle();

		// normal texture
		sceneImages.push_back(core.createImage(vk::Format::eR8G8B8A8Unorm, normalTexture.w, normalTexture.h, 1, true, true));
		sceneImages.back().fill(normalTexture.data.data());
		sceneImages.back().recordMipChainGeneration(mipStream, spdDownsampler);
		const vkcv::ImageHandle normalHandle = sceneImages.back().getHandle();

		// specular texture
		sceneImages.push_back(core.createImage(vk::Format::eR8G8B8A8Unorm, specularTexture.w, specularTexture.h, 1, true, true));
		sceneImages.back().fill(specularTexture.data.data());
		sceneImages.back().recordMipChainGeneration(mipStream, spdDownsampler);
		const vkcv::ImageHandle specularHandle = sceneImages.back().getHandle();

		vkcv::DescriptorWrites setWrites;
		setWrites.writeSampledImage(
				0, albedoHandle
		).writeSampledImage(
				2, normalHandle
		).writeSampledImage(
				3, specularHandle
		);
		
		setWrites.writeSampler(1, colorSampler);
		core.writeDescriptorSet(materialDescriptorSets.back(), setWrites);
	}
	
	core.submitCommandStream(mipStream, false);

	std::vector<vkcv::DescriptorSetLayoutHandle> perMeshDescriptorSetLayouts;
	std::vector<vkcv::DescriptorSetHandle> perMeshDescriptorSets;
	for (const auto& vertexGroup : scene.vertexGroups) {
	    perMeshDescriptorSetLayouts.push_back(materialDescriptorSetLayouts[vertexGroup.materialIndex]);
		perMeshDescriptorSets.push_back(materialDescriptorSets[vertexGroup.materialIndex]);
	}

	// prepass pipeline
	vkcv::DescriptorSetLayoutHandle prepassDescriptorSetLayout = core.createDescriptorSetLayout({});
	vkcv::DescriptorSetHandle prepassDescriptorSet = core.createDescriptorSet(prepassDescriptorSetLayout);

	auto swapchainExtent = core.getSwapchain(windowHandle).getExtent();
	
	vkcv::GraphicsPipelineConfig prepassPipelineConfig{
		depthPrepassShader,
		swapchainExtent.width,
		swapchainExtent.height,
		prepassPass,
		vertexLayout,
		{ prepassDescriptorSetLayout, perMeshDescriptorSetLayouts[0] },
		true
	};
	prepassPipelineConfig.m_culling         = vkcv::CullMode::Back;
	prepassPipelineConfig.m_multisampling   = msaa;
	prepassPipelineConfig.m_depthTest       = vkcv::DepthTest::LessEqual;
	prepassPipelineConfig.m_alphaToCoverage = true;

	vkcv::GraphicsPipelineHandle prepassPipeline = core.createGraphicsPipeline(prepassPipelineConfig);

	// forward pipeline
	vkcv::GraphicsPipelineConfig forwardPipelineConfig {
		forwardProgram,
		swapchainExtent.width,
		swapchainExtent.height,
		forwardPass,
		vertexLayout,
		{ forwardShadingDescriptorSetLayout, perMeshDescriptorSetLayouts[0] },
		true
	};
    forwardPipelineConfig.m_culling         = vkcv::CullMode::Back;
	forwardPipelineConfig.m_multisampling   = msaa;
	forwardPipelineConfig.m_depthTest       = vkcv::DepthTest::Equal;
	forwardPipelineConfig.m_depthWrite      = false;
	
	vkcv::GraphicsPipelineHandle forwardPipeline = core.createGraphicsPipeline(forwardPipelineConfig);
	
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
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("assets/shaders/sky.vert"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		skyShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("assets/shaders/sky.frag"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		skyShader.addShader(shaderStage, path);
	});

	vkcv::GraphicsPipelineConfig skyPipeConfig;
	skyPipeConfig.m_ShaderProgram       = skyShader;
	skyPipeConfig.m_Width               = swapchainExtent.width;
	skyPipeConfig.m_Height              = swapchainExtent.height;
	skyPipeConfig.m_PassHandle          = skyPass;
	skyPipeConfig.m_VertexLayout        = vkcv::VertexLayout();
	skyPipeConfig.m_DescriptorLayouts   = {};
	skyPipeConfig.m_UseDynamicViewport  = true;
	skyPipeConfig.m_multisampling       = msaa;
	skyPipeConfig.m_depthWrite          = false;

	vkcv::GraphicsPipelineHandle skyPipe = core.createGraphicsPipeline(skyPipeConfig);

	// render targets
	vkcv::ImageHandle depthBuffer           = core.createImage(
			depthBufferFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, false, false, msaa
	).getHandle();

    const bool colorBufferRequiresStorage   = !usingMsaa;
	vkcv::ImageHandle colorBuffer           = core.createImage(
			colorBufferFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, colorBufferRequiresStorage, true, msaa
	).getHandle();

	vkcv::ImageHandle resolvedColorBuffer;
	if (usingMsaa) {
		resolvedColorBuffer = core.createImage(
				colorBufferFormat,
				swapchainExtent.width,
				swapchainExtent.height,
				1, false, true, true
		).getHandle();
	}
	else {
		resolvedColorBuffer = colorBuffer;
	}
	
	vkcv::ImageHandle swapBuffer = core.createImage(
			colorBufferFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, true
	).getHandle();
	
	vkcv::ImageHandle swapBuffer2 = core.createImage(
			colorBufferFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, true
	).getHandle();

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	bool renderVoxelVis = false;
	window.e_key.add([&renderVoxelVis](int key ,int scancode, int action, int mods) {
		if (key == GLFW_KEY_V && action == GLFW_PRESS) {
			renderVoxelVis = !renderVoxelVis;
		}
	});

	bool renderUI = true;
	window.e_key.add([&renderUI](int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_I && action == GLFW_PRESS) {
			renderUI = !renderUI;
		}
	});

	// tonemapping compute shader
	vkcv::ShaderProgram tonemappingProgram;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "assets/shaders/tonemapping.comp", 
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		tonemappingProgram.addShader(shaderStage, path);
	});

	vkcv::DescriptorSetLayoutHandle tonemappingDescriptorSetLayout = core.createDescriptorSetLayout(
	        tonemappingProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle tonemappingDescriptorSet = core.createDescriptorSet(tonemappingDescriptorSetLayout);
	vkcv::ComputePipelineHandle tonemappingPipeline = core.createComputePipeline({
		tonemappingProgram,
		{ tonemappingDescriptorSetLayout }
	});
	
	// tonemapping compute shader
	vkcv::ShaderProgram postEffectsProgram;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "assets/shaders/postEffects.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		postEffectsProgram.addShader(shaderStage, path);
	});

	vkcv::DescriptorSetLayoutHandle postEffectsDescriptorSetLayout = core.createDescriptorSetLayout(
	        postEffectsProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle postEffectsDescriptorSet = core.createDescriptorSet(postEffectsDescriptorSetLayout);
	vkcv::ComputePipelineHandle postEffectsPipeline = core.createComputePipeline({
			postEffectsProgram,
			{ postEffectsDescriptorSetLayout }
	});

	// resolve compute shader
	vkcv::ShaderProgram resolveProgram;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "assets/shaders/msaa4XResolve.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		resolveProgram.addShader(shaderStage, path);
	});

	vkcv::DescriptorSetLayoutHandle resolveDescriptorSetLayout = core.createDescriptorSetLayout(
		resolveProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle resolveDescriptorSet = core.createDescriptorSet(resolveDescriptorSetLayout);
	vkcv::ComputePipelineHandle resolvePipeline = core.createComputePipeline({
		resolveProgram,
		{ resolveDescriptorSetLayout }
	});

	vkcv::SamplerHandle resolveSampler = core.createSampler(
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerMipmapMode::NEAREST,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE
	);

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
	for (size_t i = 0; i < scene.vertexGroups.size(); i++) {
		vkcv::Mesh mesh(vertexBufferBindings[i], indexBuffers[i].getVulkanHandle(), scene.vertexGroups[i].numIndices);
		meshes.push_back(mesh);
	}

	std::vector<vkcv::DrawcallInfo> drawcalls;
	std::vector<vkcv::DrawcallInfo> prepassDrawcalls;
	for (size_t i = 0; i < meshes.size(); i++) {

		drawcalls.push_back(vkcv::DrawcallInfo(meshes[i], { 
			vkcv::DescriptorSetUsage(0, forwardShadingDescriptorSet),
			vkcv::DescriptorSetUsage(1, perMeshDescriptorSets[i]) }));
		prepassDrawcalls.push_back(vkcv::DrawcallInfo(meshes[i], {
			vkcv::DescriptorSetUsage(0, prepassDescriptorSet),
			vkcv::DescriptorSetUsage(1, perMeshDescriptorSets[i]) }));
	}

	vkcv::SamplerHandle voxelSampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE
	);

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

	vkcv::effects::BloomAndFlaresEffect bloomFlares (core, true);
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
	forwardDescriptorWrites.writeUniformBuffer(
			0, shadowMapping.getLightInfoBuffer()
	).writeUniformBuffer(
			3, cameraPosBuffer.getHandle()
	).writeUniformBuffer(
			6, voxelization.getVoxelInfoBufferHandle()
	).writeUniformBuffer(
			7, volumetricSettingsBuffer.getHandle()
	);
	
	forwardDescriptorWrites.writeSampledImage(
			1, shadowMapping.getShadowMap()
	).writeSampledImage(
			4, voxelization.getVoxelImageHandle()
	);
	
	forwardDescriptorWrites.writeSampler(
			2, shadowMapping.getShadowSampler()
	).writeSampler(
			5, voxelSampler
	);
	
	core.writeDescriptorSet(forwardShadingDescriptorSet, forwardDescriptorWrites);

	vkcv::upscaling::FSRUpscaling upscaling (core);
	uint32_t fsrWidth = swapchainExtent.width, fsrHeight = swapchainExtent.height;
	
	vkcv::upscaling::FSRQualityMode fsrMode = vkcv::upscaling::FSRQualityMode::NONE;
	int fsrModeIndex = static_cast<int>(fsrMode);
	
	const std::vector<const char*> fsrModeNames = {
			"None",
			"Ultra Quality",
			"Quality",
			"Balanced",
			"Performance"
	};
	
	bool fsrMipLoadBiasFlag = true;
	bool fsrMipLoadBiasFlagBackup = fsrMipLoadBiasFlag;
	
	vkcv::upscaling::BilinearUpscaling upscaling1 (core);
	vkcv::upscaling::NISUpscaling upscaling2 (core);
	
	const std::vector<const char*> modeNames = {
			"Bilinear Upscaling",
			"FSR Upscaling",
			"NIS Upscaling"
	};
	
	int upscalingMode = 0;
	
	vkcv::gui::GUI gui(core, windowHandle);

	glm::vec2   lightAnglesDegree               = glm::vec2(90.f, 0.f);
	glm::vec3   lightColor                      = glm::vec3(1);
	float       lightStrength                   = 25.f;
	float       maxShadowDistance               = 30.f;

	int     voxelVisualisationMip   = 0;
	float   voxelizationExtent      = 35.f;

	bool msaaCustomResolve = true;

	glm::vec3   scatteringColor     = glm::vec3(1);
	float       scatteringDensity   = 0.005;
	glm::vec3   absorptionColor     = glm::vec3(1);
	float       absorptionDensity   = 0.005;
	float       volumetricAmbient   = 0.2;

	auto start = std::chrono::system_clock::now();
	const auto appStartTime = start;
	while (vkcv::Window::hasOpenWindow()) {
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight, windowHandle)) {
			continue;
		}
		
		uint32_t width, height;
		vkcv::upscaling::getFSRResolution(
				fsrMode,
				swapchainWidth, swapchainHeight,
				width, height
		);

		if ((width != fsrWidth) || ((height != fsrHeight)) || (fsrMipLoadBiasFlagBackup != fsrMipLoadBiasFlag)) {
			fsrWidth = width;
			fsrHeight = height;
			fsrMipLoadBiasFlagBackup = fsrMipLoadBiasFlag;
			
			colorSampler = core.createSampler(
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerFilterType::LINEAR,
					vkcv::SamplerMipmapMode::LINEAR,
					vkcv::SamplerAddressMode::REPEAT,
					fsrMipLoadBiasFlag? vkcv::upscaling::getFSRLodBias(fsrMode) : 0.0f
			);
			
			for (size_t i = 0; i < scene.materials.size(); i++) {
				vkcv::DescriptorWrites setWrites;
				setWrites.writeSampler(1, colorSampler);
				core.writeDescriptorSet(materialDescriptorSets[i], setWrites);
			}
			
			depthBuffer = core.createImage(
					depthBufferFormat,
					fsrWidth, fsrHeight, 1,
					false, false, false,
					msaa
			).getHandle();
			
			colorBuffer = core.createImage(
					colorBufferFormat,
					fsrWidth, fsrHeight, 1,
					false, colorBufferRequiresStorage, true,
					msaa
			).getHandle();

			if (usingMsaa) {
				resolvedColorBuffer = core.createImage(
						colorBufferFormat,
						fsrWidth, fsrHeight, 1,
						false, true, true
				).getHandle();
			} else {
				resolvedColorBuffer = colorBuffer;
			}
			
			swapBuffer = core.createImage(
					colorBufferFormat,
					fsrWidth, fsrHeight, 1,
					false, true
			).getHandle();
			
			swapBuffer2 = core.createImage(
					colorBufferFormat,
					swapchainWidth, swapchainHeight, 1,
					false, true
			).getHandle();
		}

		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		// update descriptor sets which use swapchain image
		vkcv::DescriptorWrites tonemappingDescriptorWrites;
		tonemappingDescriptorWrites.writeSampledImage(0, resolvedColorBuffer);
		tonemappingDescriptorWrites.writeSampler(1, colorSampler);
		tonemappingDescriptorWrites.writeStorageImage(2, swapBuffer);

		core.writeDescriptorSet(tonemappingDescriptorSet, tonemappingDescriptorWrites);
		
		// update descriptor sets which use swapchain image
		vkcv::DescriptorWrites postEffectsDescriptorWrites;
		postEffectsDescriptorWrites.writeSampledImage(0, swapBuffer2);
		postEffectsDescriptorWrites.writeSampler(1, colorSampler);
		postEffectsDescriptorWrites.writeStorageImage(2, swapchainInput);
		
		core.writeDescriptorSet(postEffectsDescriptorSet, postEffectsDescriptorWrites);

		// update resolve descriptor, color images could be changed
		vkcv::DescriptorWrites resolveDescriptorWrites;
		resolveDescriptorWrites.writeSampledImage(0, colorBuffer);
		resolveDescriptorWrites.writeSampler(1, resolveSampler);
		resolveDescriptorWrites.writeStorageImage(2, resolvedColorBuffer);
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
			voxelization.getVoxelExtent(),
			windowHandle,
			spdDownsampler
		);

		// voxelization
		voxelization.setVoxelExtent(voxelizationExtent);
		voxelization.voxelizeMeshes(
			cmdStream,
			meshes, 
			modelMatrices,
			perMeshDescriptorSets,
			windowHandle,
			spdDownsampler
		);

		// depth prepass
		const glm::mat4 viewProjectionCamera = cameraManager.getActiveCamera().getMVP();
		
		vkcv::PushConstants prepassPushConstants (sizeof(glm::mat4));
		
		std::vector<glm::mat4> prepassMatrices;
		for (const auto& m : modelMatrices) {
			prepassPushConstants.appendDrawcall(viewProjectionCamera * m);
		}

		
		const std::vector<vkcv::ImageHandle>    prepassRenderTargets = { depthBuffer };

		core.recordBeginDebugLabel(cmdStream, "Depth prepass", { 1, 1, 1, 1 });
		core.recordDrawcallsToCmdStream(
			cmdStream,
			prepassPass,
			prepassPipeline,
			prepassPushConstants,
			prepassDrawcalls,
			prepassRenderTargets,
			windowHandle);

		core.recordImageMemoryBarrier(cmdStream, depthBuffer);
		core.recordEndDebugLabel(cmdStream);
		
		vkcv::PushConstants pushConstants (2 * sizeof(glm::mat4));
		
		// main pass
		for (const auto& m : modelMatrices) {
			pushConstants.appendDrawcall(std::array<glm::mat4, 2>{ viewProjectionCamera * m, m });
		}

		VolumetricSettings volumeSettings;
		volumeSettings.scatteringCoefficient    = scatteringColor * scatteringDensity;
		volumeSettings.absorptionCoefficient    = absorptionColor * absorptionDensity;
		volumeSettings.ambientLight             = volumetricAmbient;
		volumetricSettingsBuffer.fill({ volumeSettings });
		
		const std::vector<vkcv::ImageHandle>    renderTargets = { colorBuffer, depthBuffer };

		core.recordBeginDebugLabel(cmdStream, "Forward rendering", { 1, 1, 1, 1 });
		core.recordDrawcallsToCmdStream(
			cmdStream,
			forwardPass,
			forwardPipeline,
			pushConstants,
			drawcalls,
			renderTargets,
			windowHandle);
		core.recordEndDebugLabel(cmdStream);

		if (renderVoxelVis) {
			voxelization.renderVoxelVisualisation(cmdStream, viewProjectionCamera, renderTargets, voxelVisualisationMip, windowHandle);
		}
		
		vkcv::PushConstants skySettingsPushConstants (sizeof(skySettings));
		skySettingsPushConstants.appendDrawcall(skySettings);

		// sky
		core.recordBeginDebugLabel(cmdStream, "Sky", { 1, 1, 1, 1 });
		core.recordDrawcallsToCmdStream(
			cmdStream,
			skyPass,
			skyPipe,
			skySettingsPushConstants,
			{ vkcv::DrawcallInfo(vkcv::Mesh({}, nullptr, 3), {}) },
			renderTargets,
			windowHandle);
		core.recordEndDebugLabel(cmdStream);

		const uint32_t fullscreenLocalGroupSize = 8;
		
		uint32_t fulsscreenDispatchCount [3];
		
		fulsscreenDispatchCount[0] = static_cast<uint32_t>(
				glm::ceil(fsrWidth  / static_cast<float>(fullscreenLocalGroupSize))
		);
		
		fulsscreenDispatchCount[1] = static_cast<uint32_t>(
				glm::ceil(fsrHeight / static_cast<float>(fullscreenLocalGroupSize))
		);
		
		fulsscreenDispatchCount[2] = 1;

		if (usingMsaa) {
			core.recordBeginDebugLabel(cmdStream, "MSAA resolve", { 1, 1, 1, 1 });
			if (msaaCustomResolve) {
				core.prepareImageForSampling(cmdStream, colorBuffer);
				core.prepareImageForStorage(cmdStream, resolvedColorBuffer);

				assert(msaa == vkcv::Multisampling::MSAA4X);	// shaders is written for msaa 4x
				core.recordComputeDispatchToCmdStream(
					cmdStream,
					resolvePipeline,
					fulsscreenDispatchCount,
					{ vkcv::DescriptorSetUsage(0, resolveDescriptorSet) },
					vkcv::PushConstants(0));

				core.recordImageMemoryBarrier(cmdStream, resolvedColorBuffer);
			}
			else {
				core.resolveMSAAImage(cmdStream, colorBuffer, resolvedColorBuffer);
			}
			core.recordEndDebugLabel(cmdStream);
		}
		
		bloomFlares.updateCameraDirection(cameraManager.getActiveCamera());
		bloomFlares.recordEffect(cmdStream, resolvedColorBuffer, resolvedColorBuffer);

		core.prepareImageForStorage(cmdStream, swapBuffer);
		core.prepareImageForSampling(cmdStream, resolvedColorBuffer);
		
		core.recordBeginDebugLabel(cmdStream, "Tonemapping", { 1, 1, 1, 1 });
		core.recordComputeDispatchToCmdStream(
			cmdStream, 
			tonemappingPipeline, 
			fulsscreenDispatchCount,
			{ vkcv::DescriptorSetUsage(0, tonemappingDescriptorSet) },
			vkcv::PushConstants(0)
		);
		
		core.prepareImageForStorage(cmdStream, swapBuffer2);
		core.prepareImageForSampling(cmdStream, swapBuffer);
		core.recordEndDebugLabel(cmdStream);
		
		switch (upscalingMode) {
			case 0:
				upscaling1.recordUpscaling(cmdStream, swapBuffer, swapBuffer2);
				break;
			case 1:
				upscaling.recordUpscaling(cmdStream, swapBuffer, swapBuffer2);
				break;
			case 2:
				upscaling2.recordUpscaling(cmdStream, swapBuffer, swapBuffer2);
				break;
			default:
				break;
		}
		
		core.prepareImageForStorage(cmdStream, swapchainInput);
		core.prepareImageForSampling(cmdStream, swapBuffer2);
		
		auto timeSinceStart = std::chrono::duration_cast<std::chrono::microseconds>(end - appStartTime);
		float timeF         = static_cast<float>(timeSinceStart.count()) * 0.01f;
		
		vkcv::PushConstants timePushConstants (sizeof(timeF));
		timePushConstants.appendDrawcall(timeF);
		
		fulsscreenDispatchCount[0] = static_cast<uint32_t>(
				glm::ceil(swapchainWidth  / static_cast<float>(fullscreenLocalGroupSize))
		);
		
		fulsscreenDispatchCount[1] = static_cast<uint32_t>(
				glm::ceil(swapchainHeight / static_cast<float>(fullscreenLocalGroupSize))
		);
		
		core.recordBeginDebugLabel(cmdStream, "Post Processing", { 1, 1, 1, 1 });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				postEffectsPipeline,
				fulsscreenDispatchCount,
				{ vkcv::DescriptorSetUsage(0, postEffectsDescriptorSet) },
				timePushConstants
		);
		core.recordEndDebugLabel(cmdStream);

		// present and end
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		// draw UI
		gui.beginGUI();

		if (renderUI) {
			ImGui::Begin("Settings");

			ImGui::Checkbox("MSAA custom resolve", &msaaCustomResolve);

			ImGui::DragFloat2("Light angles", &lightAnglesDegree.x);
			ImGui::ColorEdit3("Sun color", &lightColor.x);
			ImGui::DragFloat("Sun strength", &lightStrength);
			ImGui::DragFloat("Max shadow distance", &maxShadowDistance);
			maxShadowDistance = std::max(maxShadowDistance, 1.f);

			ImGui::ColorEdit3("Sky color", &skySettings.color.x);
			ImGui::DragFloat("Sky strength", &skySettings.strength, 0.1);

			ImGui::Checkbox("Draw voxel visualisation", &renderVoxelVis);
			ImGui::SliderInt("Visualisation mip", &voxelVisualisationMip, 0, 7);
			ImGui::DragFloat("Voxelization extent", &voxelizationExtent, 1.f, 0.f);
			voxelizationExtent = std::max(voxelizationExtent, 1.f);
			voxelVisualisationMip = std::max(voxelVisualisationMip, 0);
			
			ImGui::ColorEdit3("Scattering color", &scatteringColor.x);
			ImGui::DragFloat("Scattering density", &scatteringDensity, 0.0001);
			ImGui::ColorEdit3("Absorption color", &absorptionColor.x);
			ImGui::DragFloat("Absorption density", &absorptionDensity, 0.0001);
			ImGui::DragFloat("Volumetric ambient", &volumetricAmbient, 0.002);
			
			float sharpness = upscaling.getSharpness();
			
			ImGui::Combo("FSR Quality Mode", &fsrModeIndex, fsrModeNames.data(), fsrModeNames.size());
			ImGui::DragFloat("FSR Sharpness", &sharpness, 0.001, 0.0f, 1.0f);
			ImGui::Checkbox("FSR Mip Lod Bias", &fsrMipLoadBiasFlag);
			ImGui::Combo("Upscaling Mode", &upscalingMode, modeNames.data(), modeNames.size());
			
			if ((fsrModeIndex >= 0) && (fsrModeIndex <= 4)) {
				fsrMode = static_cast<vkcv::upscaling::FSRQualityMode>(fsrModeIndex);
			}
			
			upscaling.setSharpness(sharpness);
			upscaling2.setSharpness(sharpness);

			if (ImGui::Button("Reload forward pass")) {

				vkcv::ShaderProgram newForwardProgram;
				compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("assets/shaders/shader.vert"),
					[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
					newForwardProgram.addShader(shaderStage, path);
				});
				compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("assets/shaders/shader.frag"),
					[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
					newForwardProgram.addShader(shaderStage, path);
				});
				forwardPipelineConfig.m_ShaderProgram = newForwardProgram;
				vkcv::GraphicsPipelineHandle newPipeline = core.createGraphicsPipeline(forwardPipelineConfig);

				if (newPipeline) {
					forwardPipeline = newPipeline;
				}
			}
			if (ImGui::Button("Reload tonemapping")) {

				vkcv::ShaderProgram newProgram;
				compiler.compile(vkcv::ShaderStage::COMPUTE, std::filesystem::path("assets/shaders/tonemapping.comp"),
					[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
					newProgram.addShader(shaderStage, path);
				});

				vkcv::ComputePipelineHandle newPipeline = core.createComputePipeline({
					newProgram,
					{ tonemappingDescriptorSetLayout }
				});

				if (newPipeline) {
					tonemappingPipeline = newPipeline;
				}
			}
			ImGui::End();
		}

		gui.endGUI();

		core.endFrame(windowHandle);
	}
	
	return 0;
}
