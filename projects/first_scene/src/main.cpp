#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/Logger.hpp>
#include <vkcv/scene/Scene.hpp>



int main(int argc, const char** argv) {
	const char* applicationName = "First Scene";

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

	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -3));
	cameraManager.getCamera(camIndex0).setNearFar(0.1f, 30.0f);
	
	cameraManager.getCamera(camIndex1).setNearFar(0.1f, 30.0f);

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		{},
		{ "VK_KHR_swapchain" }
	);
	
	vkcv::scene::Scene scene = vkcv::scene::Scene::load(core, std::filesystem::path(
			argc > 1 ? argv[1] : "resources/Sponza/Sponza.gltf"
	));

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

	vkcv::ShaderProgram sceneShaderProgram{};
	sceneShaderProgram.addShader(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/vert.spv"));
	sceneShaderProgram.addShader(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/frag.spv"));

	const std::vector<vkcv::VertexAttachment> vertexAttachments = sceneShaderProgram.getVertexAttachments();
	std::vector<vkcv::VertexBinding> bindings;
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::VertexBinding(i, { vertexAttachments[i] }));
	}

	const vkcv::VertexLayout sceneLayout(bindings);
	
	const auto& material0 = scene.getMaterial(0);

	const vkcv::PipelineConfig scenePipelineDefsinition{
		sceneShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		scenePass,
		{sceneLayout},
		{ core.getDescriptorSet(material0.getDescriptorSet()).layout },
		true };
	vkcv::PipelineHandle scenePipeline = core.createGraphicsPipeline(scenePipelineDefsinition);
	
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

		scene.recordDrawcalls(cmdStream,
							  cameraManager.getActiveCamera(),
							  scenePass,
							  scenePipeline,
							  renderTargets);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame();
	}
	
	return 0;
}
