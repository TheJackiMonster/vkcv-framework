#include <iostream>
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Pass.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/gui/GUI.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/scene/Scene.hpp>
#include <vkcv/effects/BloomAndFlaresEffect.hpp>
#include <vkcv/upscaling/FSRUpscaling.hpp>

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
	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, windowWidth, windowHeight, true);
	vkcv::Window& window = core.getWindow(windowHandle);
	vkcv::camera::CameraManager cameraManager(window);
	
	vkcv::gui::GUI gui (core, windowHandle);
	
	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	uint32_t camIndex1 = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(15.5f, 0, 0));
	cameraManager.getCamera(camIndex0).setNearFar(0.1f, 30.0f);
	
	cameraManager.getCamera(camIndex1).setNearFar(0.1f, 30.0f);
	
	vkcv::scene::Scene scene = vkcv::scene::Scene::load(core, std::filesystem::path(
			argc > 1 ? argv[1] : "assets/skull_scaled/scene.gltf"
	));
	
	vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
	
	vkcv::PassHandle linePass = vkcv::passFormats(core, { colorFormat, vk::Format::eD32Sfloat });
	vkcv::PassHandle scenePass = vkcv::passFormats(core, { colorFormat, vk::Format::eD32Sfloat }, false);
	
	if ((!scenePass) || (!linePass)) {
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	vkcv::ShaderProgram sceneShaderProgram;
	vkcv::ShaderProgram lineShaderProgram;
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compileProgram(sceneShaderProgram, {
			{ vkcv::ShaderStage::VERTEX, "assets/shaders/shader.vert" },
			{ vkcv::ShaderStage::GEOMETRY, "assets/shaders/shader.geom" },
			{ vkcv::ShaderStage::FRAGMENT, "assets/shaders/shader.frag" }
	}, nullptr);
	
	compiler.compileProgram(lineShaderProgram, {
			{ vkcv::ShaderStage::VERTEX, "assets/shaders/shader.vert" },
			{ vkcv::ShaderStage::GEOMETRY, "assets/shaders/wired.geom" },
			{ vkcv::ShaderStage::FRAGMENT, "assets/shaders/red.frag" }
	}, nullptr);
	
	const std::vector<vkcv::VertexAttachment> vertexAttachments = sceneShaderProgram.getVertexAttachments();
	std::vector<vkcv::VertexBinding> bindings;
	
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::createVertexBinding(i, { vertexAttachments[i] }));
	}
	
	const auto& clipBindings = sceneShaderProgram.getReflectedDescriptors().at(1);
	
	auto clipDescriptorSetLayout = core.createDescriptorSetLayout(clipBindings);
	auto clipDescriptorSet = core.createDescriptorSet(clipDescriptorSetLayout);
	
	float clipLimit = 1.0f;
	float clipX = 0.0f;
	float clipY = 0.0f;
	float clipZ = 0.0f;
	
	auto clipBuffer = vkcv::buffer<float>(core, vkcv::BufferType::UNIFORM, 4);
	clipBuffer.fill({ clipLimit, -clipX, -clipY, -clipZ });
	
	vkcv::DescriptorWrites clipWrites;
	clipWrites.writeUniformBuffer(0, clipBuffer.getHandle());
	
	core.writeDescriptorSet(clipDescriptorSet, clipWrites);
	
	float mouseX = -0.0f;
	bool dragLimit = false;
	
	window.e_mouseMove.add([&](double x, double y) {
		double cx = (x - window.getWidth() * 0.5);
		double dx = cx / window.getWidth();
		
		mouseX = 2.0f * static_cast<float>(dx);
		
		if (dragLimit) {
			clipLimit = mouseX;
		}
	});
	
	window.e_mouseButton.add([&](int button, int action, int mods) {
		if ((std::abs(mouseX - clipLimit) < 0.1f) && (action == GLFW_PRESS)) {
			dragLimit = true;
		} else {
			dragLimit = false;
		}
	});
	
	const vkcv::VertexLayout sceneLayout { bindings };
	const auto& material0 = scene.getMaterial(0);
	
	vkcv::GraphicsPipelineHandle scenePipeline = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
				sceneShaderProgram,
				scenePass,
				{ sceneLayout },
				{ material0.getDescriptorSetLayout(), clipDescriptorSetLayout }
			)
	);
	
	vkcv::GraphicsPipelineHandle linePipeline = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
					lineShaderProgram,
					linePass,
					{ sceneLayout },
					{ material0.getDescriptorSetLayout(), clipDescriptorSetLayout }
			)
	);
	
	if ((!scenePipeline) || (!linePipeline)) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	auto swapchainExtent = core.getSwapchainExtent(window.getSwapchain());
	
	vkcv::ImageHandle depthBuffer = core.createImage(
			vk::Format::eD32Sfloat,
			swapchainExtent.width,
			swapchainExtent.height
	).getHandle();
	
	vkcv::ImageHandle colorBuffer = core.createImage(
			colorFormat,
			swapchainExtent.width,
			swapchainExtent.height,
			1, false, true, true
	).getHandle();
	
	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	
	vkcv::effects::BloomAndFlaresEffect bloomAndFlares (core);
	vkcv::upscaling::FSRUpscaling upscaling (core);
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((swapchainWidth != swapchainExtent.width) || ((swapchainHeight != swapchainExtent.height))) {
			depthBuffer = core.createImage(vk::Format::eD32Sfloat, swapchainWidth, swapchainHeight).getHandle();
			
			colorBuffer = core.createImage(
					colorFormat,
					swapchainExtent.width,
					swapchainExtent.height,
					1, false, true, true
			).getHandle();
			
			swapchainExtent.width = swapchainWidth;
			swapchainExtent.height = swapchainHeight;
		}

		cameraManager.update(dt);
		
		clipBuffer.fill({ clipLimit, -clipX, -clipY, -clipZ });
		
		const std::vector<vkcv::ImageHandle> renderTargets = { colorBuffer, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		
		auto recordMesh = [&](const glm::mat4& MVP, const glm::mat4& M,
							 vkcv::PushConstants &pushConstants,
							 vkcv::DrawcallInfo& drawcallInfo) {
			pushConstants.appendDrawcall(MVP);
			drawcallInfo.descriptorSets.push_back(
					vkcv::DescriptorSetUsage(1, clipDescriptorSet)
			);
		};
		
		scene.recordDrawcalls(cmdStream,
							  cameraManager.getActiveCamera(),
							  linePass,
							  linePipeline,
							  sizeof(glm::mat4),
							  recordMesh,
							  renderTargets,
							  windowHandle);
		
		bloomAndFlares.recordEffect(cmdStream, colorBuffer, colorBuffer);
		
		scene.recordDrawcalls(cmdStream,
							  cameraManager.getActiveCamera(),
							  scenePass,
							  scenePipeline,
							  sizeof(glm::mat4),
							  recordMesh,
							  renderTargets,
							  windowHandle);
		
		core.prepareImageForSampling(cmdStream, colorBuffer);
		core.prepareImageForStorage(cmdStream, swapchainInput);
		upscaling.recordUpscaling(cmdStream, colorBuffer, swapchainInput);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Clip X", &clipX, -1.0f, 1.0f);
		ImGui::SliderFloat("Clip Y", &clipY, -1.0f, 1.0f);
		ImGui::SliderFloat("Clip Z", &clipZ, -1.0f, 1.0f);
		ImGui::Text("Mesh by HannahNewey (https://sketchfab.com/HannahNewey)");
		ImGui::End();
		
		gui.endGUI();
	});
	
	return 0;
}
