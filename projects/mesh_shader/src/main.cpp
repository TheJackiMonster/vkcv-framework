#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>

#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/gui/GUI.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "Mesh shader";

	const int windowWidth = 1280;
	const int windowHeight = 720;
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
		{ "VK_KHR_swapchain", VK_NV_MESH_SHADER_EXTENSION_NAME }
	);
	
	vkcv::gui::GUI gui (core, window);

	const auto& context = core.getContext();
	const vk::Instance& instance = context.getInstance();
	const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
	const vk::Device& device = context.getDevice();

	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchain().getFormat());

	vkcv::PassConfig trianglePassDefinition({ present_color_attachment });
	vkcv::PassHandle renderPass = core.createPass(trianglePassDefinition);

	if (!renderPass)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram triangleShaderProgram{};
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compile(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/shader.vert"),
					 [&triangleShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		 triangleShaderProgram.addShader(shaderStage, path);
	});
	
	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
					 [&triangleShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		triangleShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::PipelineConfig trianglePipelineDefinition {
		triangleShaderProgram,
		(uint32_t)windowWidth,
		(uint32_t)windowHeight,
		renderPass,
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

	// mesh shader
	vkcv::ShaderProgram meshShaderProgram;
	compiler.compile(vkcv::ShaderStage::TASK, std::filesystem::path("resources/shaders/shader.task"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::MESH, std::filesystem::path("resources/shaders/shader.mesh"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

	compiler.compile(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/shader.frag"),
		[&meshShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		meshShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::PipelineConfig meshShaderPipelineDefinition{
		meshShaderProgram,
		(uint32_t)windowWidth,
		(uint32_t)windowHeight,
		renderPass,
		{},
		{},
		false
	};

	vkcv::PipelineHandle meshShaderPipeline = core.createGraphicsPipeline(meshShaderPipelineDefinition);

	if (!meshShaderPipeline)
	{
		std::cout << "Error. Could not create mesh shader pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	auto start = std::chrono::system_clock::now();

	vkcv::ImageHandle swapchainImageHandle = vkcv::ImageHandle::createSwapchainImageHandle();

	const vkcv::Mesh renderMesh({}, nullptr, 3);
	vkcv::DrawcallInfo drawcall(renderMesh, {});

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    vkcv::camera::CameraManager cameraManager(window);
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));

	while (window.isWindowOpen())
	{
        window.pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}
		
        auto end = std::chrono::system_clock::now();
        auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        start = end;
		
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
        glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();

		vkcv::PushConstantData pushConstantData((void*)&mvp, sizeof(glm::mat4));
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordMeshShaderDrawcalls(
			cmdStream,
			renderPass,
			meshShaderPipeline,
			vkcv::PushConstantData(nullptr, 0),
			{ vkcv::MeshShaderDrawcall({}, 1) },
			{ swapchainInput });

		core.recordDrawcallsToCmdStream(
			cmdStream,
			renderPass,
			trianglePipeline,
			pushConstantData,
			{ drawcall },
			{ swapchainInput });


		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		// gui.beginGUI();
		// 
		// ImGui::Begin("Settings");
		// ImGui::End();
		// 
		// gui.endGUI();
	    
	    core.endFrame();
	}
	return 0;
}
