#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "First Mesh";

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

	const char* path = argc > 1 ? argv[1] : "resources/cube/cube.gltf";
	int result = vkcv::asset::loadScene(path, mesh);

	if (result == 1) {
		std::cout << "Mesh loading successful!" << std::endl;
	}
	else {
		std::cout << "Mesh loading failed: " << result << std::endl;
		return 1;
	}

	assert(mesh.vertexGroups.size() > 0);
	auto vertexBuffer = core.createBuffer<uint8_t>(
			vkcv::BufferType::VERTEX,
			mesh.vertexGroups[0].vertexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	vertexBuffer.fill(mesh.vertexGroups[0].vertexBuffer.data);

	auto indexBuffer = core.createBuffer<uint8_t>(
			vkcv::BufferType::INDEX,
			mesh.vertexGroups[0].indexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	indexBuffer.fill(mesh.vertexGroups[0].indexBuffer.data);
	
	auto& vertexAttributes = mesh.vertexGroups[0].vertexBuffer.attributes;
	
	std::sort(vertexAttributes.begin(), vertexAttributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
		return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
	});

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
		vkcv::VertexBufferBinding(vertexAttributes[0].offset, vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(vertexAttributes[1].offset, vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(vertexAttributes[2].offset, vertexBuffer.getVulkanHandle()) };

	const vkcv::Mesh loadedMesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), mesh.vertexGroups[0].numIndices);

	// an example attachment for passes that output to the window
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

	vkcv::PassConfig firstMeshPassDefinition({ present_color_attachment, depth_attachment });
	vkcv::PassHandle firstMeshPass = core.createPass(firstMeshPassDefinition);

	if (!firstMeshPass) {
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::shader::GLSLCompiler compiler;

	vkcv::ShaderProgram firstMeshProgram;
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"), 
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		firstMeshProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
		[&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		firstMeshProgram.addShader(shaderStage, path);
	});

	const std::vector<vkcv::VertexAttachment> vertexAttachments = firstMeshProgram.getVertexAttachments();

    std::vector<vkcv::VertexBinding> bindings;
    for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
    }
    
    const vkcv::VertexLayout firstMeshLayout (bindings);

	std::vector<vkcv::DescriptorBinding> descriptorBindings = { firstMeshProgram.getReflectedDescriptors()[0] };
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

	const vkcv::PipelineConfig firstMeshPipelineConfig {
        firstMeshProgram,
		windowWidth,
		windowHeight,
        firstMeshPass,
        firstMeshLayout,
		{ core.getDescriptorSet(descriptorSet).layout },
		true
	};
	
	vkcv::PipelineHandle firstMeshPipeline = core.createGraphicsPipeline(firstMeshPipelineConfig);
	
	if (!firstMeshPipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	//vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Srgb, mesh.texture_hack.w, mesh.texture_hack.h);
	//texture.fill(mesh.texture_hack.img);
    vkcv::asset::Texture &tex = mesh.textures[0];
    vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Srgb, tex.w, tex.h);
    texture.fill(tex.data.data());

	vkcv::SamplerHandle sampler = core.createSampler(
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
        firstMeshLayout,
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

	vkcv::DescriptorWrites setWrites;
	setWrites.sampledImageWrites    = { 
        vkcv::SampledImageDescriptorWrite(0, texture.getHandle()),
        vkcv::SampledImageDescriptorWrite(3, shadowMap.getHandle()) };
	setWrites.samplerWrites         = { 
        vkcv::SamplerDescriptorWrite(1, sampler), 
        vkcv::SamplerDescriptorWrite(4, shadowSampler) };
    setWrites.uniformBufferWrites   = { vkcv::UniformBufferDescriptorWrite(2, lightBuffer.getHandle()) };
	core.writeDescriptorSet(descriptorSet, setWrites);

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
		firstMeshLayout,
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

		const float shadowProjectionSize = 5.f;
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

		// compute positions and transform matrices
		std::vector<glm::vec3> instancePositions = {
		glm::vec3(0.f, -2.f, 0.f),
		glm::vec3(3.f,  2 * glm::sin(duration.count() * 0.0025), 0.f),
		glm::vec3(-3.f,  0.f, 0.f),
		glm::vec3(0.f,  2.f, 0.f),
		glm::vec3(0.f, -5.f, 0.f)
		};

		std::vector<glm::mat4> modelMatrices;
		std::vector<vkcv::DrawcallInfo> drawcalls;
		std::vector<vkcv::DrawcallInfo> shadowDrawcalls;
		std::vector<vkcv::DrawcallInfo> voxelizationDrawcalls;
		for (const auto& position : instancePositions) {
			modelMatrices.push_back(glm::translate(glm::mat4(1.f), position));
			drawcalls.push_back(vkcv::DrawcallInfo(loadedMesh, { descriptorUsage }));
			shadowDrawcalls.push_back(vkcv::DrawcallInfo(loadedMesh, {}));
			voxelizationDrawcalls.push_back(vkcv::DrawcallInfo(loadedMesh, { voxelizationDescriptorUsage }));
		}
		modelMatrices.back() *= glm::scale(glm::mat4(1.f), glm::vec3(10.f, 1.f, 10.f));

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
            firstMeshPass,
            firstMeshPipeline,
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
