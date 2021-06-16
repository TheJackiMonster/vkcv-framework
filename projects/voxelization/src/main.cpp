#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/Logger.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "Voxelization";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;
	
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

	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchainImageFormat()
	);
	
	const vk::Format depthBufferFormat = vk::Format::eD32Sfloat;
	const vkcv::AttachmentDescription depth_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		depthBufferFormat
	);

	vkcv::PassConfig forwardPassDefinition({ present_color_attachment, depth_attachment });
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

	std::vector<vkcv::DescriptorBinding> descriptorBindings = { forwardProgram.getReflectedDescriptors()[0] };
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

	const vkcv::PipelineConfig forwardPipelineConfig {
        forwardProgram,
		windowWidth,
		windowHeight,
        forwardPass,
		vertexLayout,
		{ core.getDescriptorSet(descriptorSet).layout },
		true
	};
	
	vkcv::PipelineHandle forwardPipeline = core.createGraphicsPipeline(forwardPipelineConfig);
	
	if (!forwardPipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	vkcv::SamplerHandle colorSampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::REPEAT
	);

	vkcv::SamplerHandle shadowSampler = core.createSampler(
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerMipmapMode::NEAREST,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE
	);

	vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight).getHandle();

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	vkcv::ShaderProgram shadowShader;
	compiler.compile(vkcv::ShaderStage::VERTEX, "resources/shaders/shadow.vert",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shadowShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "resources/shaders/shadow.frag",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		shadowShader.addShader(shaderStage, path);
	});

	const vk::Format shadowMapFormat = vk::Format::eD16Unorm;
	const std::vector<vkcv::AttachmentDescription> shadowAttachments = {
		vkcv::AttachmentDescription(vkcv::AttachmentOperation::STORE, vkcv::AttachmentOperation::CLEAR, shadowMapFormat)
	};
	const vkcv::PassConfig shadowPassConfig(shadowAttachments);
	const vkcv::PassHandle shadowPass = core.createPass(shadowPassConfig);

	const uint32_t shadowMapResolution = 1024;
	const vkcv::Image shadowMap = core.createImage(shadowMapFormat, shadowMapResolution, shadowMapResolution, 1);
	const vkcv::PipelineConfig shadowPipeConfig {
		shadowShader, 
		shadowMapResolution, 
		shadowMapResolution, 
		shadowPass,
		vertexLayout,
		{},
		false
	};
	
	const vkcv::PipelineHandle shadowPipe = core.createGraphicsPipeline(shadowPipeConfig);

	struct LightInfo {
		glm::vec3 direction;
		float padding;
		glm::mat4 lightMatrix;
	};
	LightInfo lightInfo;
	vkcv::Buffer lightBuffer = core.createBuffer<LightInfo>(vkcv::BufferType::UNIFORM, sizeof(glm::vec3));

	const uint32_t voxelResolution = 32;
	vkcv::Image voxelImage = core.createImage(vk::Format::eR8Unorm, voxelResolution, voxelResolution, voxelResolution, true);
	
	const vk::Format voxelizationDummyFormat = vk::Format::eR8Unorm;
	vkcv::Image voxelizationDummyRenderTarget = core.createImage(voxelizationDummyFormat, voxelResolution, voxelResolution, 1, false, true);

	vkcv::ShaderProgram voxelizationShader;
	compiler.compile(vkcv::ShaderStage::VERTEX, "resources/shaders/voxelization.vert",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelizationShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::GEOMETRY, "resources/shaders/voxelization.geom",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelizationShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "resources/shaders/voxelization.frag",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelizationShader.addShader(shaderStage, path);
	});

	vkcv::PassConfig voxelizationPassConfig({
		vkcv::AttachmentDescription(vkcv::AttachmentOperation::DONT_CARE, vkcv::AttachmentOperation::DONT_CARE, voxelizationDummyFormat)});
	vkcv::PassHandle voxelizationPass = core.createPass(voxelizationPassConfig);

	std::vector<vkcv::DescriptorBinding> voxelizationDescriptorBindings = { voxelizationShader.getReflectedDescriptors()[0] };
	vkcv::DescriptorSetHandle voxelizationDescriptorSet = core.createDescriptorSet(voxelizationDescriptorBindings);

	const vkcv::PipelineConfig voxelizationPipeConfig{
		voxelizationShader,
		voxelResolution,
		voxelResolution,
		voxelizationPass,
		vertexLayout,
		{ core.getDescriptorSet(voxelizationDescriptorSet).layout },
		false,
		true };
	const vkcv::PipelineHandle voxelizationPipe = core.createGraphicsPipeline(voxelizationPipeConfig);

	struct VoxelizationInfo {
		glm::vec3 offset;
		float extent;
	};
	vkcv::Buffer voxelizationInfoBuffer = core.createBuffer<VoxelizationInfo>(vkcv::BufferType::UNIFORM, sizeof(VoxelizationInfo));

	vkcv::DescriptorWrites voxelizationDescriptorWrites;
	voxelizationDescriptorWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(0, voxelImage.getHandle()) };
	voxelizationDescriptorWrites.uniformBufferWrites = { vkcv::UniformBufferDescriptorWrite(1, voxelizationInfoBuffer.getHandle()) };
	core.writeDescriptorSet(voxelizationDescriptorSet, voxelizationDescriptorWrites);

	const size_t voxelCount = voxelResolution * voxelResolution * voxelResolution;

	vkcv::ShaderProgram voxelVisualisationShader;
	compiler.compile(vkcv::ShaderStage::VERTEX, "resources/shaders/voxelVisualisation.vert",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelVisualisationShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::GEOMETRY, "resources/shaders/voxelVisualisation.geom",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelVisualisationShader.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "resources/shaders/voxelVisualisation.frag",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelVisualisationShader.addShader(shaderStage, path);
	});

	const std::vector<vkcv::DescriptorBinding> voxelVisualisationDescriptorBindings = { voxelVisualisationShader.getReflectedDescriptors()[0] };
	vkcv::DescriptorSetHandle voxelVisualisationDescriptorSet = core.createDescriptorSet(voxelVisualisationDescriptorBindings);

	const vkcv::AttachmentDescription voxelVisualisationColorAttachments(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		core.getSwapchainImageFormat()
	);

	const vkcv::AttachmentDescription voxelVisualisationDepthAttachments(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::LOAD,
		depthBufferFormat
	);

	vkcv::PassConfig voxelVisualisationPassDefinition({ voxelVisualisationColorAttachments, voxelVisualisationDepthAttachments });
	vkcv::PassHandle voxelVisualisationPass = core.createPass(voxelVisualisationPassDefinition);

	const vkcv::PipelineConfig voxelVisualisationPipeConfig{
		voxelVisualisationShader,
		0,
		0,
		voxelVisualisationPass,
		{},
		{ core.getDescriptorSet(voxelVisualisationDescriptorSet).layout },
		true,
		false,
		vkcv::PrimitiveTopology::PointList };	// points are extended to cubes in the geometry shader
	const vkcv::PipelineHandle voxelVisualisationPipe = core.createGraphicsPipeline(voxelVisualisationPipeConfig);

	vkcv::Buffer<uint16_t> voxelVisualisationIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, voxelCount);
	std::vector<uint16_t> voxelIndexData;
	for (int i = 0; i < voxelCount; i++) {
		voxelIndexData.push_back(i);
	}

	vkcv::DescriptorWrites voxelVisualisationDescriptorWrite;
	voxelVisualisationDescriptorWrite.storageImageWrites  = { vkcv::StorageImageDescriptorWrite(0,  voxelImage.getHandle()) };
	voxelVisualisationDescriptorWrite.uniformBufferWrites = { vkcv::UniformBufferDescriptorWrite(1, voxelizationInfoBuffer.getHandle()) };
	core.writeDescriptorSet(voxelVisualisationDescriptorSet, voxelVisualisationDescriptorWrite);

	voxelVisualisationIndexBuffer.fill(voxelIndexData);
	const vkcv::DrawcallInfo voxelVisualisationDrawcall(
		vkcv::Mesh({}, voxelVisualisationIndexBuffer.getVulkanHandle(), voxelCount),
		{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(voxelVisualisationDescriptorSet).vulkanHandle) });

	const vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);
	const vkcv::DescriptorSetUsage voxelizationDescriptorUsage(0, core.getDescriptorSet(voxelizationDescriptorSet).vulkanHandle);

	std::vector<std::array<glm::mat4, 2>> mainPassMatrices;
	std::vector<glm::mat4> mvpLight;
	std::vector<std::array<glm::mat4, 2>> voxelizationMatrices;

	bool renderVoxelVis = false;
	window.e_key.add([&renderVoxelVis](int key ,int scancode, int action, int mods) {
		if (key == GLFW_KEY_V && action == GLFW_PRESS) {
			renderVoxelVis = !renderVoxelVis;
		}
	});

	vkcv::ShaderProgram resetVoxelShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "resources/shaders/voxelReset.comp",
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		resetVoxelShader.addShader(shaderStage, path);
	});

	vkcv::DescriptorSetHandle resetVoxelDescriptorSet = core.createDescriptorSet(resetVoxelShader.getReflectedDescriptors()[0]);
	vkcv::PipelineHandle resetVoxelPipeline = core.createComputePipeline(
		resetVoxelShader, 
		{  core.getDescriptorSet(resetVoxelDescriptorSet).layout });

	vkcv::DescriptorWrites resetVoxelWrites;
	resetVoxelWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(0, voxelImage.getHandle()) };
	core.writeDescriptorSet(resetVoxelDescriptorSet, resetVoxelWrites);

	// prepare descriptor sets for drawcalls
	std::vector<vkcv::Image> sceneImages;
	std::vector<vkcv::DescriptorSetHandle> descriptorSets;
	for (const auto& vertexGroup : scene.vertexGroups) {
		descriptorSets.push_back(core.createDescriptorSet(descriptorBindings));

		const auto& material = scene.materials[vertexGroup.materialIndex];

		int baseColorIndex = material.baseColor;
		if (baseColorIndex < 0) {
			vkcv_log(vkcv::LogLevel::WARNING, "Material lacks base color");
			baseColorIndex = 0;
		}

		vkcv::asset::Texture& sceneTexture = scene.textures[baseColorIndex];

		sceneImages.push_back(core.createImage(vk::Format::eR8G8B8A8Srgb, sceneTexture.w, sceneTexture.h));
		sceneImages.back().fill(sceneTexture.data.data());

		vkcv::DescriptorWrites setWrites;
		setWrites.sampledImageWrites = {
			vkcv::SampledImageDescriptorWrite(0, sceneImages.back().getHandle()),
			vkcv::SampledImageDescriptorWrite(3, shadowMap.getHandle())
		};
		setWrites.samplerWrites = {
			vkcv::SamplerDescriptorWrite(1, colorSampler),
			vkcv::SamplerDescriptorWrite(4, shadowSampler),
		};
		setWrites.uniformBufferWrites = { vkcv::UniformBufferDescriptorWrite(2, lightBuffer.getHandle()) };
		core.writeDescriptorSet(descriptorSets.back(), setWrites);
	}

	// model matrices per mesh
	std::vector<glm::mat4> modelMatrices;
	modelMatrices.resize(scene.vertexGroups.size(), glm::mat4(1.f));
	for (const auto& mesh : scene.meshes) {
		const glm::mat4 m = *reinterpret_cast<const glm::mat4*>(&mesh.modelMatrix[0]);
		for (const auto& vertexGroupIndex : mesh.vertexGroups) {
			modelMatrices[vertexGroupIndex] = m;
		}
	}

	auto start = std::chrono::system_clock::now();
	const auto appStartTime = start;
	while (window.isWindowOpen()) {
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}

		if ((swapchainWidth != windowWidth) || ((swapchainHeight != windowHeight))) {
			depthBuffer = core.createImage(depthBufferFormat, swapchainWidth, swapchainHeight).getHandle();

			windowWidth = swapchainWidth;
			windowHeight = swapchainHeight;
		}

		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		
		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - appStartTime);
		
		const float sunTheta = 0.001f * static_cast<float>(duration.count());
		lightInfo.direction = glm::normalize(glm::vec3(std::cos(sunTheta), 1, std::sin(sunTheta)));

		const float shadowProjectionSize = 20.f;
		glm::mat4 projectionLight = glm::ortho(
			-shadowProjectionSize,
			shadowProjectionSize,
			-shadowProjectionSize,
			shadowProjectionSize,
			-shadowProjectionSize,
			shadowProjectionSize);

		glm::mat4 vulkanCorrectionMatrix(1.f);
		vulkanCorrectionMatrix[2][2] = 0.5;
		vulkanCorrectionMatrix[3][2] = 0.5;
		projectionLight = vulkanCorrectionMatrix * projectionLight;

		const glm::mat4 viewLight = glm::lookAt(glm::vec3(0), -lightInfo.direction, glm::vec3(0, -1, 0));

		lightInfo.lightMatrix = projectionLight * viewLight;
		lightBuffer.fill({ lightInfo });

		const glm::mat4 viewProjectionCamera = cameraManager.getActiveCamera().getMVP();

		const float voxelizationExtent = 20.f;
		VoxelizationInfo voxelizationInfo;
		voxelizationInfo.extent = voxelizationExtent;

		// move voxel offset with camera in voxel sized steps
		const glm::vec3 cameraPos = cameraManager.getActiveCamera().getPosition();
		const float voxelSize = voxelizationExtent / voxelResolution;
		voxelizationInfo.offset = glm::floor(cameraPos / voxelSize) * voxelSize;

		voxelizationInfoBuffer.fill({ voxelizationInfo });

		const float voxelizationHalfExtent = 0.5f * voxelizationExtent;
		const glm::mat4 voxelizationProjection = glm::ortho(
			-voxelizationHalfExtent,
			voxelizationHalfExtent,
			-voxelizationHalfExtent,
			voxelizationHalfExtent,
			-voxelizationHalfExtent,
			voxelizationHalfExtent);

		const glm::mat4 voxelizationView = glm::translate(glm::mat4(1.f), -voxelizationInfo.offset);
		const glm::mat4 voxelizationViewProjection = voxelizationProjection * voxelizationView;

		std::vector<vkcv::DrawcallInfo> drawcalls;
		std::vector<vkcv::DrawcallInfo> shadowDrawcalls;
		std::vector<vkcv::DrawcallInfo> voxelizationDrawcalls;
		for (int i = 0; i < scene.vertexGroups.size(); i++) {
			vkcv::Mesh mesh(vertexBufferBindings[i], indexBuffers[i].getVulkanHandle(), scene.vertexGroups[i].numIndices);

			vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(descriptorSets[i]).vulkanHandle);

			drawcalls.push_back(vkcv::DrawcallInfo(mesh, { descriptorUsage }));
			shadowDrawcalls.push_back(vkcv::DrawcallInfo(mesh, {}));
			voxelizationDrawcalls.push_back(vkcv::DrawcallInfo(mesh, { voxelizationDescriptorUsage }));
		}

		mainPassMatrices.clear();
		mvpLight.clear();
		voxelizationMatrices.clear();
		for (const auto& m : modelMatrices) {
			mainPassMatrices.push_back({ viewProjectionCamera * m, m });
			mvpLight.push_back(lightInfo.lightMatrix * m);
			voxelizationMatrices.push_back({ voxelizationViewProjection * m, m });
		}

		vkcv::PushConstantData pushConstantData((void*)mainPassMatrices.data(), 2 * sizeof(glm::mat4));
		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };

		const vkcv::PushConstantData shadowPushConstantData((void*)mvpLight.data(), sizeof(glm::mat4));
		const vkcv::PushConstantData voxelizationPushConstantData((void*)voxelizationMatrices.data(), 2 * sizeof(glm::mat4));

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		// shadow map
		core.recordDrawcallsToCmdStream(
			cmdStream,
			shadowPass,
			shadowPipe,
			shadowPushConstantData,
			shadowDrawcalls,
			{ shadowMap.getHandle() });

		// reset voxels
		const uint32_t resetVoxelGroupSize[3] = { 4, 4, 4 };
		uint32_t resetVoxelDispatchCount[3];
		for(int i = 0; i < 3; i++) {
			resetVoxelDispatchCount[i] = glm::ceil(voxelResolution / float(resetVoxelGroupSize[i]));
		}

		core.prepareImageForStorage(cmdStream, voxelImage.getHandle());
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			resetVoxelPipeline,
			resetVoxelDispatchCount,
			{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(resetVoxelDescriptorSet).vulkanHandle) },
			vkcv::PushConstantData(nullptr, 0));
		core.recordImageMemoryBarrier(cmdStream, voxelImage.getHandle());

		// voxelization
		core.recordDrawcallsToCmdStream(
			cmdStream,
			voxelizationPass,
			voxelizationPipe,
			voxelizationPushConstantData,
			voxelizationDrawcalls,
			{ voxelizationDummyRenderTarget.getHandle() });

		core.prepareImageForSampling(cmdStream, shadowMap.getHandle());

		// main pass
		core.recordDrawcallsToCmdStream(
			cmdStream,
            forwardPass,
            forwardPipeline,
			pushConstantData,
			drawcalls,
			renderTargets);

		if (renderVoxelVis) {
			const vkcv::PushConstantData voxelVisualisationPushConstantData((void*)&viewProjectionCamera, sizeof(glm::mat4));

			core.recordImageMemoryBarrier(cmdStream, voxelImage.getHandle());
			core.recordDrawcallsToCmdStream(
				cmdStream,
				voxelVisualisationPass,
				voxelVisualisationPipe,
				voxelVisualisationPushConstantData,
				{ voxelVisualisationDrawcall },
				renderTargets);
		}

		// present and end
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		core.endFrame();
	}
	
	return 0;
}
