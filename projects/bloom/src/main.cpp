#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>

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

	vkcv::CameraManager cameraManager(window, windowWidth, windowHeight);
	cameraManager.getCamera().setPosition(glm::vec3(0.f, 0.f, 3.f));
	cameraManager.getCamera().setNearFar(0.1, 30);

	window.initEvents();

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		{},
		{ "VK_KHR_swapchain" }
	);


    // ATTACHMENTS
    const vkcv::AttachmentDescription offscreenAttachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            vk::Format::eB10G11R11UfloatPack32
    );

    const vkcv::AttachmentDescription shadowAttachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            vk::Format::eD16Unorm
    );

    const vkcv::AttachmentDescription depthAttachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            vk::Format::eD32Sfloat
    );

    const vkcv::AttachmentDescription presentAttachment(
            vkcv::AttachmentOperation::STORE,
            vkcv::AttachmentOperation::CLEAR,
            core.getSwapchainImageFormat()
    );

    // RENDER PASSES
    const vkcv::PassConfig shadowPassConfig({shadowAttachment});
    const vkcv::PassHandle shadowPassHandle = core.createPass(shadowPassConfig);

    const vkcv::PassConfig meshPassDefinition({ offscreenAttachment, depthAttachment });
    const vkcv::PassHandle meshPassHandle = core.createPass(meshPassDefinition);

    const vkcv::PassConfig screenQuadPassConfig({presentAttachment});
    const vkcv::PassHandle screenQuadPassHandle = core.createPass(screenQuadPassConfig);

    if (!shadowPassHandle || !meshPassHandle || !screenQuadPassHandle)
    {
        std::cout << "Error. Could not create render passes. Exiting." << std::endl;
        return EXIT_FAILURE;
    }


    // SHADERS
    vkcv::ShaderProgram shadowProgram;
    shadowProgram.addShader(vkcv::ShaderStage::VERTEX, "resources/shaders/shadow_vert.spv");
    shadowProgram.addShader(vkcv::ShaderStage::FRAGMENT, "resources/shaders/shadow_frag.spv");

    vkcv::ShaderProgram meshProgram{};
    meshProgram.addShader(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/vert.spv"));
    meshProgram.addShader(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/frag.spv"));

    vkcv::ShaderProgram bloomComputeProgram{};
    bloomComputeProgram.addShader(vkcv::ShaderStage::COMPUTE, std::filesystem::path("resources/shaders/blur_comp.spv"));

    vkcv::ShaderProgram screenQuadProgram{};
    screenQuadProgram.addShader(vkcv::ShaderStage::VERTEX, "resources/shaders/quad_vert.spv");
    screenQuadProgram.addShader(vkcv::ShaderStage::FRAGMENT, "resources/shaders/quad_frag.spv");


    vkcv::asset::Mesh mesh;

	const char* path = argc > 1 ? argv[1] : "resources/cube/cube.gltf";
	int result = vkcv::asset::loadMesh(path, mesh);

	if (result == 1) {
		std::cout << "Mesh loading successful!" << std::endl;
	}
	else {
		std::cout << "Mesh loading failed: " << result << std::endl;
		return 1;
	}
	assert(mesh.vertexGroups.size() > 0);

    auto& attributes = mesh.vertexGroups[0].vertexBuffer.attributes;

    std::sort(attributes.begin(), attributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
        return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
    });

    // MESH INDEX/VERTEX BUFFER
	auto meshVertexBuffer = core.createBuffer<uint8_t>(
			vkcv::BufferType::VERTEX,
			mesh.vertexGroups[0].vertexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
    meshVertexBuffer.fill(mesh.vertexGroups[0].vertexBuffer.data);

	auto meshIndexBuffer = core.createBuffer<uint8_t>(
			vkcv::BufferType::INDEX,
			mesh.vertexGroups[0].indexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
    meshIndexBuffer.fill(mesh.vertexGroups[0].indexBuffer.data);

    // MESH VERTEX LAYOUT
    const std::vector<vkcv::VertexBufferBinding> meshVertexBufferBindings = {
            vkcv::VertexBufferBinding(attributes[0].offset, meshVertexBuffer.getVulkanHandle()),
            vkcv::VertexBufferBinding(attributes[1].offset, meshVertexBuffer.getVulkanHandle()),
            vkcv::VertexBufferBinding(attributes[2].offset, meshVertexBuffer.getVulkanHandle()) };

    const std::vector<vkcv::VertexAttachment> meshVertexAttachments = meshProgram.getVertexAttachments();
    std::vector<vkcv::VertexBinding> meshBindings;
    for (size_t i = 0; i < meshVertexAttachments.size(); i++) {
        meshBindings.push_back(vkcv::VertexBinding(i, { meshVertexAttachments[i] }));
    }
    const vkcv::VertexLayout meshVertexLayout(meshBindings);

    const vkcv::Mesh loadedMesh(meshVertexBufferBindings, meshIndexBuffer.getVulkanHandle(), mesh.vertexGroups[0].numIndices);

    // QUAD INDEX/VERTEX BUFFER
	const std::vector<float> quadFloats = {-1.0f,  1.0f, 0.0f,  // Left Bottom
                                            1.0f,  1.0f, 0.0f,  // Right Bottom
                                           -1.0f, -1.0f, 0.0f,  // Left Top
                                            1.0f, -1.0f, 0.0f}; // Right Top

    const std::vector<uint8_t> quadIndices = {0, 1, 2,
                                              2, 1, 3};

    auto quadVertexBuffer = core.createBuffer<float>(
            vkcv::BufferType::VERTEX,
            quadFloats.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    quadVertexBuffer.fill(quadFloats);

    auto quadIndexBuffer = core.createBuffer<uint8_t>(
            vkcv::BufferType::INDEX,
            quadIndices.size(),
            vkcv::BufferMemoryType::DEVICE_LOCAL);
    quadIndexBuffer.fill(quadIndices);

	// SCREEN QUAD VERTEX LAYOUT
	const std::vector<vkcv::VertexBufferBinding> quadVertexBufferBindings = {
	        vkcv::VertexBufferBinding(0, quadVertexBuffer.getVulkanHandle())
	};
	const vkcv::VertexBinding quadBinding(0, screenQuadProgram.getVertexAttachments());
	const vkcv::VertexLayout quadLayout({quadBinding});

    const vkcv::Mesh loadedQuad(quadVertexBufferBindings, quadIndexBuffer.getVulkanHandle(), quadIndices.size());

	// RESOURCES
    vkcv::Image meshTexture = core.createImage(vk::Format::eR8G8B8A8Srgb, mesh.texture_hack.w, mesh.texture_hack.h);
    meshTexture.fill(mesh.texture_hack.img);
    const vkcv::ImageHandle meshTextureHandle = meshTexture.getHandle();

    vkcv::ImageHandle depthBufferHandle = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight).getHandle();

    vkcv::Image offscreenImage = core.createImage(vk::Format::eB10G11R11UfloatPack32, windowWidth, windowHeight, 1);
    vkcv::ImageHandle bloomImageHandle = core.createImage(vk::Format::eB10G11R11UfloatPack32, windowWidth, windowHeight, 1).getHandle();

    const vkcv::ImageHandle swapchainHandle = vkcv::ImageHandle::createSwapchainImageHandle();


    const vkcv::ImageHandle shadowMapHandle = core.createImage(vk::Format::eD16Unorm, 1024, 1024, 1).getHandle();

    const vkcv::SamplerHandle linearSampler = core.createSampler(
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerFilterType::LINEAR,
            vkcv::SamplerMipmapMode::LINEAR,
            vkcv::SamplerAddressMode::REPEAT
    );

    const vkcv::SamplerHandle shadowSampler = core.createSampler(
            vkcv::SamplerFilterType::NEAREST,
            vkcv::SamplerFilterType::NEAREST,
            vkcv::SamplerMipmapMode::NEAREST,
            vkcv::SamplerAddressMode::CLAMP_TO_EDGE
    );


    struct LightInfo {
        glm::vec3 direction;
        float padding;
        glm::mat4 lightMatrix;
    };
    LightInfo lightInfo{};
    vkcv::Buffer lightBuffer = core.createBuffer<LightInfo>(vkcv::BufferType::UNIFORM, sizeof(glm::vec3));

    // PIPELINES & DESCRIPTOR STUFF
	const std::vector<vkcv::DescriptorBinding> meshDescriptorBindings = { meshProgram.getReflectedDescriptors()[0] };
	const vkcv::DescriptorSetHandle meshDescriptorSet = core.createDescriptorSet(meshDescriptorBindings);
    const vkcv::DescriptorSetUsage meshDescriptorUsage(0, core.getDescriptorSet(meshDescriptorSet).vulkanHandle);

    vkcv::DescriptorWrites meshDescriptorWrites;
    meshDescriptorWrites.sampledImageWrites    = {
            vkcv::SampledImageDescriptorWrite(0, meshTextureHandle),
            vkcv::SampledImageDescriptorWrite(3, shadowMapHandle) };
    meshDescriptorWrites.samplerWrites         = {
            vkcv::SamplerDescriptorWrite(1, linearSampler),
            vkcv::SamplerDescriptorWrite(4, shadowSampler) };
    meshDescriptorWrites.uniformBufferWrites   = { vkcv::UniformBufferDescriptorWrite(2, lightBuffer.getHandle()) };
    core.writeDescriptorSet(meshDescriptorSet, meshDescriptorWrites);

    const vkcv::PipelineConfig meshPipelineConfig(
        meshProgram,
		windowWidth,
		windowHeight,
        meshPassHandle,
        {meshVertexLayout},
		{ core.getDescriptorSet(meshDescriptorSet).layout },
		true);
	vkcv::PipelineHandle meshPipelineHandle = core.createGraphicsPipeline(meshPipelineConfig);

	// --

    const vkcv::PipelineConfig shadowPipeConfig(
            shadowProgram,
            1024,
            1024,
            shadowPassHandle,
            {meshVertexLayout},
            {},
            false);
    const vkcv::PipelineHandle shadowPipelineHandle = core.createGraphicsPipeline(shadowPipeConfig);

    // --

    const std::vector<vkcv::DescriptorBinding> bloomSetBindings = bloomComputeProgram.getReflectedDescriptors()[0];
    const vkcv::DescriptorSetHandle bloomSetHandle = core.createDescriptorSet(bloomSetBindings);
    const vkcv::DescriptorSetUsage bloomSetUsage(0, core.getDescriptorSet(bloomSetHandle).vulkanHandle);

    vkcv::DescriptorWrites bloomSetWrites;
    bloomSetWrites.sampledImageWrites  = {vkcv::SampledImageDescriptorWrite(0, offscreenImage.getHandle())};
    bloomSetWrites.samplerWrites       = {vkcv::SamplerDescriptorWrite(1, linearSampler)};
    bloomSetWrites.storageImageWrites  = {vkcv::StorageImageDescriptorWrite(2, bloomImageHandle)};
    core.writeDescriptorSet(bloomSetHandle, bloomSetWrites);
    vkcv::PipelineHandle bloomComputePipelineHandle = core.createComputePipeline(bloomComputeProgram, {core.getDescriptorSet(bloomSetHandle).layout});

    // --
    const std::vector<vkcv::DescriptorBinding> screenQuadBindings = { screenQuadProgram.getReflectedDescriptors()[0] };
    const vkcv::DescriptorSetHandle screenQuadSet = core.createDescriptorSet(screenQuadBindings);
    const vkcv::DescriptorSetUsage screenQuadDescriptorUsage(0, core.getDescriptorSet(screenQuadSet).vulkanHandle);

    vkcv::DescriptorWrites screenQuadSetDescriptorWrites;
    screenQuadSetDescriptorWrites.sampledImageWrites    = {
            vkcv::SampledImageDescriptorWrite(0, offscreenImage.getHandle()),
            vkcv::SampledImageDescriptorWrite(1, bloomImageHandle) };
    screenQuadSetDescriptorWrites.samplerWrites         = {
            vkcv::SamplerDescriptorWrite(2, linearSampler) };
    core.writeDescriptorSet(screenQuadSet, screenQuadSetDescriptorWrites);

    const vkcv::PipelineConfig screenQuadConfig(
            screenQuadProgram,
            windowWidth,
            windowHeight,
            screenQuadPassHandle,
            {quadLayout},
            { core.getDescriptorSet(meshDescriptorSet).layout },
            true);
    vkcv::PipelineHandle screenQuadPipelineHandle = core.createGraphicsPipeline(screenQuadConfig);

    if (!meshPipelineHandle || !shadowPipelineHandle || !bloomComputePipelineHandle) {
        std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
        return EXIT_FAILURE;
    }

    // DRAWCALLS AND MVP STUFF

	const std::vector<glm::vec3> cubeTranslations = {
		glm::vec3( 0.f, -2.f, 0.f),
		glm::vec3( 3.f,  0.f, 0.f),
		glm::vec3(-3.f,  0.f, 0.f),
		glm::vec3( 0.f,  2.f, 0.f),
		glm::vec3( 0.f, -5.f, 0.f)
	};

	std::vector<glm::mat4> modelMatrices;
	std::vector<vkcv::DrawcallInfo> meshDrawcalls;
	std::vector<vkcv::DrawcallInfo> shadowDrawcalls;
	for (const auto& translationVec : cubeTranslations) {
		modelMatrices.push_back(glm::translate(glm::mat4(1.f), translationVec));
        meshDrawcalls.push_back(vkcv::DrawcallInfo(loadedMesh, { meshDescriptorUsage }));
		shadowDrawcalls.push_back(vkcv::DrawcallInfo(loadedMesh, {}));
	}

	vkcv::DrawcallInfo quadDrawcall(loadedQuad, {screenQuadDescriptorUsage});

	modelMatrices.back() *= glm::scale(glm::mat4(1.f), glm::vec3(10.f, 1.f, 10.f));

	std::vector<std::array<glm::mat4, 2>> mainPassMatrices;
	std::vector<glm::mat4> mvpLight;


	auto start = std::chrono::system_clock::now();
	const auto appStartTime = start;
    while (window.isWindowOpen()) {
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}
		if ((swapchainWidth != windowWidth) || ((swapchainHeight != windowHeight))) {
            depthBufferHandle = core.createImage(vk::Format::eD32Sfloat, swapchainWidth, swapchainHeight).getHandle();

			windowWidth = swapchainWidth;
			windowHeight = swapchainHeight;
		}

		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		start = end;
		cameraManager.getCamera().updateView(deltatime.count() * 0.000001);

		const float sunTheta = std::chrono::duration_cast<std::chrono::milliseconds>(end - appStartTime).count() * 0.001f;
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

		const glm::mat4 viewProjectionCamera = cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView();

		mainPassMatrices.clear();
		mvpLight.clear();
		for (const auto& m : modelMatrices) {
			mainPassMatrices.push_back({ viewProjectionCamera * m, m });
			mvpLight.push_back(lightInfo.lightMatrix* m);
		}

		vkcv::PushConstantData pushConstantData((void*)mainPassMatrices.data(), 2 * sizeof(glm::mat4));

		vkcv::PushConstantData shadowPushConstantData((void*)mvpLight.data(), sizeof(glm::mat4));

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		// shadow map pass
		core.recordDrawcallsToCmdStream(
			cmdStream,
			shadowPassHandle,
			shadowPipelineHandle,
			shadowPushConstantData,
			shadowDrawcalls,
			{ shadowMapHandle });

		core.prepareImageForSampling(cmdStream, shadowMapHandle);

        // offscreen render pass
		core.recordDrawcallsToCmdStream(
			cmdStream,
            meshPassHandle,
            meshPipelineHandle,
			pushConstantData,
			meshDrawcalls,
            {offscreenImage.getHandle(), depthBufferHandle});


		core.prepareImageForSampling(cmdStream, offscreenImage.getHandle());
		//core.prepareImageForStorage(cmdStream, bloomAttachment.getHandle());

		// compute blur pass
		const std::array<uint32_t, 3> dispatchCount = {windowWidth, windowHeight, 0};
		/*
		core.recordComputeDispatchToCmdStream(
		        cmdStream,
		        bloomComputePipeline,
		        dispatchCount.data(),
                {bloomSetUsage},
                vkcv::PushConstantData(nullptr, 0));
        */
		// final compositing screen quad pass
		core.recordDrawcallsToCmdStream(
		        cmdStream,
		        screenQuadPassHandle,
		        screenQuadPipelineHandle,
		        vkcv::PushConstantData(nullptr, 0),
                {quadDrawcall},
                {swapchainHandle}
		        );

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		core.endFrame();
	}

	return 0;
}
