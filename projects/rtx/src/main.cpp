#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/scene/Scene.hpp>
#include <vkcv/rtx/RTX.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "RTX";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;

	vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		true
	);

	vkcv::camera::CameraManager cameraManager(window);
	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(-8, 1, -0.5));
	cameraManager.getCamera(camIndex0).setNearFar(0.1f, 30.0f);
	
	cameraManager.getCamera(camIndex1).setNearFar(0.1f, 30.0f);

	// prepare raytracing extensions. IMPORTANT: configure compiler to build in 64 bit mode
    vkcv::rtx::RTXModule rtxModule;
    std::vector<const char*> raytracingInstanceExtensions = rtxModule.getInstanceExtensions();

    std::vector<const char*> instanceExtensions = {};   // add some more instance extensions, if needed
    instanceExtensions.insert(instanceExtensions.end(), raytracingInstanceExtensions.begin(), raytracingInstanceExtensions.end());  // merge together all instance extensions

    vkcv::Features features = rtxModule.getFeatures();  // all features required by the RTX device extensions
    features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		features,
        instanceExtensions
	);

	vkcv::scene::Scene scene = vkcv::scene::Scene::load(core, std::filesystem::path(
			argc > 1 ? argv[1] : "resources/Sponza/Sponza.gltf"
	));

    // TODO: replace by bigger scene
	vkcv::asset::Scene mesh;

	const char* path = argc > 1 ? argv[1] : "resources/cube/cube.gltf";
	int result = vkcv::asset::loadScene(path, mesh);

	if (result == 1) {
	    std::cout << "Mesh loading successful!" << std::endl;
	} else {
	    std::cerr << "Mesh loading failed: " << result << std::endl;
	    return 1;
	}

	assert(!mesh.vertexGroups.empty());

	std::vector<uint8_t> vertices = {};
	for (size_t i=0; i<mesh.vertexGroups[0].vertexBuffer.data.size(); i++) {
	    vertices.push_back(mesh.vertexGroups[0].vertexBuffer.data[i]);
	}

	std::vector<uint8_t> indices = {};
	for (size_t i=0; i<mesh.vertexGroups[0].indexBuffer.data.size(); i++) {
	    indices.push_back(mesh.vertexGroups[0].indexBuffer.data[i]);
	}

	// init RTXModule
    rtxModule.init(&core, vertices, indices);

	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain().getFormat()
	);

	const vkcv::AttachmentDescription depth_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		vk::Format::eD32Sfloat
	);

	vkcv::PassConfig scenePassDefinition({ present_color_attachment, depth_attachment });
	vkcv::PassHandle scenePass = core.createPass(scenePassDefinition);

	if (!scenePass) {
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram sceneShaderProgram;
	vkcv::shader::GLSLCompiler compiler;

	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"),
					 [&sceneShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		sceneShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
					 [&sceneShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		sceneShaderProgram.addShader(shaderStage, path);
	});

	const std::vector<vkcv::VertexAttachment> vertexAttachments = sceneShaderProgram.getVertexAttachments();
	std::vector<vkcv::VertexBinding> bindings;
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
	}

	const vkcv::VertexLayout sceneLayout(bindings);

	const auto& material0 = scene.getMaterial(0);

	// TODO
//	vkcv::DescriptorSetLayoutHandle vertexShaderDescriptorSetLayout = core.createDescriptorSetLayout(bunnyShaderProgram.getReflectedDescriptors().at(0));
//	vkcv::DescriptorSetHandle vertexShaderDescriptorSet = core.createDescriptorSet(vertexShaderDescriptorSetLayout);

	const vkcv::PipelineConfig scenePipelineDefinition{
		sceneShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		scenePass,
		{sceneLayout},
		{ core.getDescriptorSetLayout(material0.getDescriptorSetLayout()).vulkanHandle },
		true };
	vkcv::PipelineHandle scenePipeline = core.createGraphicsPipeline(scenePipelineDefinition);

	// TODO
//	vkcv::DescriptorWrites vertexShaderDescriptorWrites;
//	vertexShaderDescriptorWrites.storageBufferWrites = { vkcv::BufferDescriptorWrite(0, matrixBuffer.getHandle()) };
//	core.writeDescriptorSet(vertexShaderDescriptorSet, vertexShaderDescriptorWrites);

	if (!scenePipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight).getHandle();

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	auto start = std::chrono::system_clock::now();
	while (window.isWindowOpen()) {
        vkcv::Window::pollEvents();

		if(window.getHeight() == 0 || window.getWidth() == 0)
			continue;

		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}

		if ((swapchainWidth != windowWidth) || ((swapchainHeight != windowHeight))) {
			depthBuffer = core.createImage(vk::Format::eD32Sfloat, swapchainWidth, swapchainHeight).getHandle();

			windowWidth = swapchainWidth;
			windowHeight = swapchainHeight;
		}

		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		auto recordMesh = [](const glm::mat4& MVP, const glm::mat4& M,
							 vkcv::PushConstants &pushConstants,
							 vkcv::DrawcallInfo& drawcallInfo) {
			pushConstants.appendDrawcall(MVP);
		};

		scene.recordDrawcalls(cmdStream,
							  cameraManager.getActiveCamera(),
							  scenePass,
							  scenePipeline,
							  sizeof(glm::mat4),
							  recordMesh,
							  renderTargets);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame();
	}
	
	return 0;
}
