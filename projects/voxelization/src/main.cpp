#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/Sampler.hpp>
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
	const std::string applicationName = "Voxelization";

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
	auto camHandle0  = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	auto camHandle1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);

	cameraManager.getCamera(camHandle0).setPosition(glm::vec3(0.f, 0.f, 3.f));
	cameraManager.getCamera(camHandle0).setNearFar(0.1f, 30.0f);
	cameraManager.getCamera(camHandle0).setYaw(180.0f);
	cameraManager.getCamera(camHandle0).setFov(glm::radians(37.8));	// fov of a 35mm lens
	
	cameraManager.getCamera(camHandle1).setNearFar(0.1f, 30.0f);

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

	std::vector<std::vector<vkcv::VertexBufferBinding>> vertexBufferBindings;

	for (size_t i = 0; i < scene.vertexGroups.size(); i++) {
		vBuffers.push_back(scene.vertexGroups[i].vertexBuffer.data);
		iBuffers.push_back(scene.vertexGroups[i].indexBuffer.data);
	}

	std::vector<vkcv::Buffer<uint8_t>> vertexBuffers;
	for (const vkcv::asset::VertexGroup& group : scene.vertexGroups) {
		vertexBuffers.push_back(buffer<uint8_t>(
			core,
			vkcv::BufferType::VERTEX,
			group.vertexBuffer.data.size()));
		vertexBuffers.back().fill(group.vertexBuffer.data);
	}

	std::vector<vkcv::Buffer<uint8_t>> indexBuffers;
	for (const auto& dataBuffer : iBuffers) {
		indexBuffers.push_back(buffer<uint8_t>(
			core,
			vkcv::BufferType::INDEX,
			dataBuffer.size()));
		indexBuffers.back().fill(dataBuffer);
	}

	for (size_t i = 0; i < scene.vertexGroups.size(); i++) {
		vertexBufferBindings.push_back(vkcv::asset::loadVertexBufferBindings(
				scene.vertexGroups[i].vertexBuffer.attributes,
				vertexBuffers[i].getHandle(),
				{
						vkcv::asset::PrimitiveType::POSITION,
						vkcv::asset::PrimitiveType::NORMAL,
						vkcv::asset::PrimitiveType::TEXCOORD_0,
						vkcv::asset::PrimitiveType::TANGENT
				}
		));
	}

	const vk::Format colorBufferFormat = vk::Format::eB10G11R11UfloatPack32;
	const vkcv::AttachmentDescription color_attachment (
			colorBufferFormat,
			vkcv::AttachmentOperation::CLEAR,
			vkcv::AttachmentOperation::STORE
	);
	
	const vk::Format depthBufferFormat = vk::Format::eD32Sfloat;
	const vkcv::AttachmentDescription depth_attachment (
			depthBufferFormat,
			vkcv::AttachmentOperation::LOAD,
			vkcv::AttachmentOperation::STORE
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
	
	vkcv::PassHandle prepassPass = vkcv::passFormat(core, depthBufferFormat, true, msaa);

	// create descriptor sets
	vkcv::SamplerHandle colorSampler = vkcv::samplerLinear(core);

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
		sceneImages.push_back(vkcv::image(core, vk::Format::eR8G8B8A8Srgb, albedoTexture.w, albedoTexture.h, 1, true));
		sceneImages.back().fill(albedoTexture.data.data());
		sceneImages.back().recordMipChainGeneration(mipStream, spdDownsampler);
		const vkcv::ImageHandle albedoHandle = sceneImages.back().getHandle();

		// normal texture
		sceneImages.push_back(vkcv::image(core, vk::Format::eR8G8B8A8Unorm, normalTexture.w, normalTexture.h, 1, true, true));
		sceneImages.back().fill(normalTexture.data.data());
		sceneImages.back().recordMipChainGeneration(mipStream, spdDownsampler);
		const vkcv::ImageHandle normalHandle = sceneImages.back().getHandle();

		// specular texture
		sceneImages.push_back(vkcv::image(core, vk::Format::eR8G8B8A8Unorm, specularTexture.w, specularTexture.h, 1, true, true));
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

	auto swapchainExtent = core.getSwapchainExtent(window.getSwapchain());
	
	vkcv::GraphicsPipelineConfig prepassPipelineConfig (
		depthPrepassShader,
		prepassPass,
		vertexLayout,
		{ prepassDescriptorSetLayout, perMeshDescriptorSetLayouts[0] }
	);
	
	prepassPipelineConfig.setCulling(vkcv::CullMode::Back);
	prepassPipelineConfig.setDepthTest(vkcv::DepthTest::LessEqual);
	prepassPipelineConfig.setWritingAlphaToCoverage(true);

	vkcv::GraphicsPipelineHandle prepassPipeline = core.createGraphicsPipeline(prepassPipelineConfig);

	// forward pipeline
	vkcv::GraphicsPipelineConfig forwardPipelineConfig (
		forwardProgram,
		forwardPass,
		vertexLayout,
		{ forwardShadingDescriptorSetLayout, perMeshDescriptorSetLayouts[0] }
	);
	
	forwardPipelineConfig.setCulling(vkcv::CullMode::Back);
	forwardPipelineConfig.setDepthTest(vkcv::DepthTest::Equal);
	forwardPipelineConfig.setWritingDepth(false);
	
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
	
	vkcv::PassHandle skyPass = vkcv::passFormats(
			core,
			{ colorBufferFormat, depthBufferFormat },
			false,
			msaa
	);

	vkcv::ShaderProgram skyShader;
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("assets/shaders/sky.vert"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		skyShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("assets/shaders/sky.frag"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		skyShader.addShader(shaderStage, path);
	});

	vkcv::GraphicsPipelineConfig skyPipeConfig (
			skyShader,
			skyPass,
			{},
			{}
	);
	
	skyPipeConfig.setWritingDepth(false);

	vkcv::GraphicsPipelineHandle skyPipe = core.createGraphicsPipeline(skyPipeConfig);

	// render targets
	vkcv::ImageHandle depthBuffer = core.createImage(
			depthBufferFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, false, false, msaa
	);

    const bool colorBufferRequiresStorage = !usingMsaa;
	vkcv::ImageHandle colorBuffer = core.createImage(
			colorBufferFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, colorBufferRequiresStorage, true, msaa
	);

	vkcv::ImageHandle resolvedColorBuffer;
	if (usingMsaa) {
		resolvedColorBuffer = core.createImage(
				colorBufferFormat,
				swapchainExtent.width,
				swapchainExtent.height,
				1, false, true, true
		);
	}
	else {
		resolvedColorBuffer = colorBuffer;
	}
	
	vkcv::ImageHandle swapBuffer = core.createImage(
			colorBufferFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, true
	);
	
	vkcv::ImageHandle swapBuffer2 = core.createImage(
			colorBufferFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, true
	);

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

	vkcv::SamplerHandle resolveSampler = vkcv::samplerNearest(core, true);

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
	std::vector<vkcv::VertexData> meshes;
	for (size_t i = 0; i < scene.vertexGroups.size(); i++) {
		vkcv::VertexData mesh (vertexBufferBindings[i]);
		mesh.setIndexBuffer(indexBuffers[i].getHandle());
		mesh.setCount(scene.vertexGroups[i].numIndices);
		meshes.push_back(mesh);
	}

	std::vector<vkcv::InstanceDrawcall> drawcalls;
	std::vector<vkcv::InstanceDrawcall> prepassDrawcalls;
	for (size_t i = 0; i < meshes.size(); i++) {
		vkcv::InstanceDrawcall drawcall (meshes[i]);
		drawcall.useDescriptorSet(0, forwardShadingDescriptorSet);
		drawcall.useDescriptorSet(1, perMeshDescriptorSets[i]);
		
		vkcv::InstanceDrawcall prepassDrawcall (meshes[i]);
		prepassDrawcall.useDescriptorSet(0, prepassDescriptorSet);
		prepassDrawcall.useDescriptorSet(1, perMeshDescriptorSets[i]);
		
		drawcalls.push_back(drawcall);
		prepassDrawcalls.push_back(prepassDrawcall);
	}

	vkcv::SamplerHandle voxelSampler = vkcv::samplerLinear(core, true);

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
	vkcv::Buffer<glm::vec3> cameraPosBuffer = buffer<glm::vec3>(core, vkcv::BufferType::UNIFORM, 1);

	struct VolumetricSettings {
		glm::vec3   scatteringCoefficient;
		float       ambientLight;
		glm::vec3   absorptionCoefficient;
	};
	vkcv::Buffer<VolumetricSettings> volumetricSettingsBuffer
		= buffer<VolumetricSettings>(core, vkcv::BufferType::UNIFORM ,1);

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
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		uint32_t width, height;
		vkcv::upscaling::getFSRResolution(
				fsrMode,
				swapchainWidth, swapchainHeight,
				width, height
		);

		if ((width != fsrWidth) || ((height != fsrHeight)) ||
			(fsrMipLoadBiasFlagBackup != fsrMipLoadBiasFlag)) {
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
			);
			
			colorBuffer = core.createImage(
					colorBufferFormat,
					fsrWidth, fsrHeight, 1,
					false, colorBufferRequiresStorage, true,
					msaa
			);

			if (usingMsaa) {
				resolvedColorBuffer = core.createImage(
						colorBufferFormat,
						fsrWidth, fsrHeight, 1,
						false, true, true
				);
			} else {
				resolvedColorBuffer = colorBuffer;
			}
			
			swapBuffer = core.createImage(
					colorBufferFormat,
					fsrWidth, fsrHeight, 1,
					false, true
			);
			
			swapBuffer2 = core.createImage(
					colorBufferFormat,
					swapchainWidth, swapchainHeight, 1,
					false, true
			);
		}

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

		cameraManager.update(dt);
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
		
		vkcv::PushConstants prepassPushConstants = vkcv::pushConstants<glm::mat4>();
		
		std::vector<glm::mat4> prepassMatrices;
		for (const auto& m : modelMatrices) {
			prepassPushConstants.appendDrawcall(viewProjectionCamera * m);
		}
		
		const std::vector<vkcv::ImageHandle>    prepassRenderTargets = { depthBuffer };

		core.recordBeginDebugLabel(cmdStream, "Depth prepass", { 1, 1, 1, 1 });
		core.recordDrawcallsToCmdStream(
			cmdStream,
			prepassPipeline,
			prepassPushConstants,
			prepassDrawcalls,
			prepassRenderTargets,
			windowHandle
		);

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
			forwardPipeline,
			pushConstants,
			drawcalls,
			renderTargets,
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);

		if (renderVoxelVis) {
			voxelization.renderVoxelVisualisation(cmdStream, viewProjectionCamera, renderTargets, voxelVisualisationMip, windowHandle);
		}
		
		vkcv::PushConstants skySettingsPushConstants = vkcv::pushConstants<SkySettings>();
		skySettingsPushConstants.appendDrawcall(skySettings);
		
		vkcv::VertexData skyData;
		skyData.setCount(3);

		// sky
		core.recordBeginDebugLabel(cmdStream, "Sky", { 1, 1, 1, 1 });
		core.recordDrawcallsToCmdStream(
			cmdStream,
			skyPipe,
			skySettingsPushConstants,
			{ vkcv::InstanceDrawcall(skyData) },
			renderTargets,
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		const uint32_t fullscreenLocalGroupSize = 8;
		auto fullscreenDispatchCount = vkcv::dispatchInvocations(
				vkcv::DispatchSize(fsrWidth, fsrHeight),
				vkcv::DispatchSize(fullscreenLocalGroupSize, fullscreenLocalGroupSize)
		);

		if (usingMsaa) {
			core.recordBeginDebugLabel(cmdStream, "MSAA resolve", { 1, 1, 1, 1 });
			if (msaaCustomResolve) {
				core.prepareImageForSampling(cmdStream, colorBuffer);
				core.prepareImageForStorage(cmdStream, resolvedColorBuffer);

				assert(msaa == vkcv::Multisampling::MSAA4X);	// shaders is written for msaa 4x
				core.recordComputeDispatchToCmdStream(
						cmdStream,
						resolvePipeline,
						fullscreenDispatchCount,
						{ vkcv::useDescriptorSet(0, resolveDescriptorSet) },
						vkcv::PushConstants(0)
				);

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
				fullscreenDispatchCount,
				{ vkcv::useDescriptorSet(0, tonemappingDescriptorSet) },
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
		
		vkcv::PushConstants timePushConstants = vkcv::pushConstants<float>();
		timePushConstants.appendDrawcall(static_cast<float>(t * 100000.0));
		
		fullscreenDispatchCount = vkcv::dispatchInvocations(
				vkcv::DispatchSize(swapchainWidth, swapchainHeight),
				vkcv::DispatchSize(fullscreenLocalGroupSize, fullscreenLocalGroupSize)
		);
		
		core.recordBeginDebugLabel(cmdStream, "Post Processing", { 1, 1, 1, 1 });
		core.recordComputeDispatchToCmdStream(
				cmdStream,
				postEffectsPipeline,
				fullscreenDispatchCount,
				{ vkcv::useDescriptorSet(0, postEffectsDescriptorSet) },
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
				
				forwardPipelineConfig.setShaderProgram(newForwardProgram);
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
	});
	
	return 0;
}
