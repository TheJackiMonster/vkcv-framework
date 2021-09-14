#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/scene/Scene.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "First Scene";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;

	vkcv::Core core = vkcv::Core::create(
			applicationName,
			VK_MAKE_VERSION(0, 0, 1),
			{vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
			{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }
	);
	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, false);
	vkcv::Window& window = core.getWindow(windowHandle);
	vkcv::camera::CameraManager cameraManager(window);

	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(-8, 1, -0.5));
	cameraManager.getCamera(camIndex0).setNearFar(0.1f, 30.0f);
	
	cameraManager.getCamera(camIndex1).setNearFar(0.1f, 30.0f);

	vkcv::scene::Scene scene = vkcv::scene::Scene::load(core, std::filesystem::path(
			argc > 1 ? argv[1] : "assets/Sponza/Sponza.gltf"
	));

	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain(windowHandle).getFormat()
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
	
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("assets/shaders/shader.vert"),
					 [&sceneShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		sceneShaderProgram.addShader(shaderStage, path);
	});
	
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("assets/shaders/shader.frag"),
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

	const vkcv::GraphicsPipelineConfig scenePipelineDefinition{
		sceneShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		scenePass,
		{sceneLayout},
		{ core.getDescriptorSetLayout(material0.getDescriptorSetLayout()).vulkanHandle },
		true };
	vkcv::GraphicsPipelineHandle scenePipeline = core.createGraphicsPipeline(scenePipelineDefinition);
	
	if (!scenePipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	auto swapchainExtent = core.getSwapchain(windowHandle).getExtent();

	vkcv::ImageHandle depthBuffer = core.createImage(
			vk::Format::eD32Sfloat,
			swapchainExtent.width,
			swapchainExtent.height
	).getHandle();

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	
	auto start = std::chrono::system_clock::now();
	while (vkcv::Window::hasOpenWindow()) {
        vkcv::Window::pollEvents();
		
		if(window.getHeight() == 0 || window.getWidth() == 0)
			continue;
		
		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight,windowHandle)) {
			continue;
		}
		
		if ((swapchainWidth != swapchainExtent.width) || ((swapchainHeight != swapchainExtent.height))) {
			depthBuffer = core.createImage(vk::Format::eD32Sfloat, swapchainWidth, swapchainHeight).getHandle();
			
			swapchainExtent.width = swapchainWidth;
			swapchainExtent.height = swapchainHeight;
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
							  renderTargets,
							  windowHandle);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame(windowHandle);
	}
	
	return 0;
}
