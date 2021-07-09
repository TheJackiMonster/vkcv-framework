#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>

#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/gui/GUI.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "First Triangle";

	const int windowWidth = 800;
	const int windowHeight = 600;
	vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		false
	);

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		{},
		{ "VK_KHR_swapchain" }
	);

	const auto& context = core.getContext();
	const vk::Instance& instance = context.getInstance();
	const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
	const vk::Device& device = context.getDevice();

	auto triangleIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3, vkcv::BufferMemoryType::DEVICE_LOCAL);
	uint16_t indices[3] = { 0, 1, 2 };
	triangleIndexBuffer.fill(&indices[0], sizeof(indices));

	// an example attachment for passes that output to the window
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain().getFormat());

	vkcv::PassConfig trianglePassDefinition({ present_color_attachment });
	vkcv::PassHandle trianglePass = core.createPass(trianglePassDefinition);

	if (!trianglePass)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram triangleShaderProgram{};
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("shaders/shader.vert"),
					 [&triangleShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		 triangleShaderProgram.addShader(shaderStage, path);
	});
	
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("shaders/shader.frag"),
					 [&triangleShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		triangleShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::PipelineConfig trianglePipelineDefinition {
		triangleShaderProgram,
		(uint32_t)windowWidth,
		(uint32_t)windowHeight,
		trianglePass,
		{},
		{},
		false
	};

	vkcv::PipelineHandle trianglePipeline = core.createGraphicsPipeline(trianglePipelineDefinition);

	if (!trianglePipeline)
	{
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	auto start = std::chrono::system_clock::now();

	const vkcv::Mesh renderMesh({}, triangleIndexBuffer.getVulkanHandle(), 3);
	vkcv::DrawcallInfo drawcall(renderMesh, {},1);

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    vkcv::camera::CameraManager cameraManager(window);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
    uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));
    cameraManager.getCamera(camIndex1).setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    cameraManager.getCamera(camIndex1).setCenter(glm::vec3(0.0f, 0.0f, -1.0f));

	while (window.isWindowOpen())
	{
        vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}
		
        auto end = std::chrono::system_clock::now();
        auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        start = end;
		
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
        glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();

		vkcv::PushConstants pushConstants (sizeof(glm::mat4));
		pushConstants.appendDrawcall(mvp);
		
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordDrawcallsToCmdStream(
			cmdStream,
			trianglePass,
			trianglePipeline,
			pushConstants,
			{ drawcall },
			{ swapchainInput });

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
	    
	    core.endFrame();
	}
	return 0;
}
