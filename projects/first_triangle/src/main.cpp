#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Pass.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/ShaderProgram.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

int main(int argc, const char** argv) {
	const std::string applicationName = "First Triangle";

	const int windowWidth = 800;
	const int windowHeight = 600;
	
	vkcv::Core core = vkcv::Core::create(
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics },
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }
	);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, true);
	vkcv::Window& window = core.getWindow(windowHandle);
	
	vkcv::PassHandle trianglePass = vkcv::passSwapchain(
			core,
			window.getSwapchain(),
			{ vk::Format::eUndefined }
	);

	if (!trianglePass) {
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	core.setDebugLabel(trianglePass, "Triangle Pass");

	vkcv::ShaderProgram triangleShaderProgram;
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compileProgram(triangleShaderProgram, {
		{ vkcv::ShaderStage::VERTEX, "shaders/shader.vert" },
		{ vkcv::ShaderStage::FRAGMENT, "shaders/shader.frag" }
	}, nullptr);

	vkcv::GraphicsPipelineHandle trianglePipeline = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
					triangleShaderProgram,
					trianglePass,
					{},
					{}
			)
	);

	if (!trianglePipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	core.setDebugLabel(trianglePipeline, "Triangle Pipeline");

	vkcv::VertexData vertexData;
	vertexData.setCount(3);
	
	vkcv::InstanceDrawcall drawcall (vertexData);

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

    vkcv::camera::CameraManager cameraManager(window);
    auto camHandle = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camHandle).setPosition(glm::vec3(0, 0, -2));

	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
			uint32_t swapchainWidth, uint32_t swapchainHeight) {
		cameraManager.update(dt);
		
		glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();
		
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		core.setDebugLabel(cmdStream, "Render Commands");
		
		core.recordDrawcallsToCmdStream(
				cmdStream,
				trianglePipeline,
				vkcv::pushConstants<glm::mat4>(mvp),
				{ drawcall },
				{ swapchainInput },
				windowHandle
		);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
	});
	
	return 0;
}
