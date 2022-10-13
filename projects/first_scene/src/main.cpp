#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Pass.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/scene/Scene.hpp>
#include <vkcv/Features.hpp>

int main(int argc, const char** argv) {
	const std::string applicationName = "First Scene";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;
	
	vkcv::Features features;
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	vkcv::Core core = vkcv::Core::create(
			applicationName,
			VK_MAKE_VERSION(0, 0, 1),
			{vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
			features
	);
	
	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, true);
	vkcv::Window& window = core.getWindow(windowHandle);
	vkcv::camera::CameraManager cameraManager(window);

    vkcv::gui::GUI gui (core, windowHandle);

    auto camHandle0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	auto camHandle1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camHandle0).setPosition(glm::vec3(-8, 1, -0.5));
	cameraManager.getCamera(camHandle0).setNearFar(0.1f, 30.0f);
	
	cameraManager.getCamera(camHandle1).setNearFar(0.1f, 30.0f);

	vkcv::scene::Scene scene = vkcv::scene::Scene::load(
			core,
			std::filesystem::path(argc > 1 ? argv[1] : "assets/Sponza/Sponza.gltf"),
			{
				vkcv::asset::PrimitiveType::POSITION,
				vkcv::asset::PrimitiveType::NORMAL,
				vkcv::asset::PrimitiveType::TEXCOORD_0
			}
	);
	
	vkcv::PassHandle scenePass = vkcv::passSwapchain(
			core,
			window.getSwapchain(),
			{ vk::Format::eUndefined, vk::Format::eD32Sfloat }
	);

	if (!scenePass) {
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram sceneShaderProgram;
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compileProgram(sceneShaderProgram, {
		{ vkcv::ShaderStage::VERTEX, "assets/shaders/shader.vert" },
		{ vkcv::ShaderStage::FRAGMENT, "assets/shaders/shader.frag" }
	}, nullptr);

	const std::vector<vkcv::VertexAttachment> vertexAttachments = sceneShaderProgram.getVertexAttachments();
	std::vector<vkcv::VertexBinding> bindings;
	
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::createVertexBinding(i, { vertexAttachments[i] }));
	}

	const vkcv::VertexLayout sceneLayout { bindings };
	const auto& material0 = scene.getMaterial(0);
	
	vkcv::GraphicsPipelineHandle scenePipeline = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
					sceneShaderProgram,
					scenePass,
					{ sceneLayout },
					{ material0.getDescriptorSetLayout() }
			)
	);
	
	if (!scenePipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	vkcv::ImageHandle depthBuffer;

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			(swapchainHeight != core.getImageHeight(depthBuffer))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					vkcv::ImageConfig(
							swapchainWidth,
							swapchainHeight
					)
			);
		}
		
		cameraManager.update(dt);

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		auto recordMesh = [](const glm::mat4& MVP, const glm::mat4& M,
							 vkcv::PushConstants &pushConstants,
							 vkcv::Drawcall& drawcall) {
			pushConstants.appendDrawcall(MVP);
		};
		
		scene.recordDrawcalls(
				cmdStream,
				cameraManager.getActiveCamera(),
				scenePass,
				scenePipeline,
				sizeof(glm::mat4),
				recordMesh,
				renderTargets,
				windowHandle
		);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

        gui.beginGUI();

        ImGui::Begin("Settings");
        ImGui::Text("Deltatime %fms, %f", dt * 1000, 1/dt);
        ImGui::End();

        gui.endGUI();
	});
	
	return 0;
}
