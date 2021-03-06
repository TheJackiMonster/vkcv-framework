#include <iostream>
#include <vkcv/Core.hpp>
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
	
	const vkcv::AttachmentDescription color_attachment0(
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::CLEAR,
			colorFormat
	);
	
	const vkcv::AttachmentDescription depth_attachment0(
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::CLEAR,
			vk::Format::eD32Sfloat
	);
	
	const vkcv::AttachmentDescription color_attachment1(
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::LOAD,
			colorFormat
	);
	
	const vkcv::AttachmentDescription depth_attachment1(
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::LOAD,
			vk::Format::eD32Sfloat
	);
	
	vkcv::PassConfig linePassDefinition(
			{ color_attachment0, depth_attachment0 },
			vkcv::Multisampling::None
	);
	
	vkcv::PassHandle linePass = core.createPass(linePassDefinition);
	
	vkcv::PassConfig scenePassDefinition(
			{ color_attachment1, depth_attachment1 },
			vkcv::Multisampling::None
	);
	
	vkcv::PassHandle scenePass = core.createPass(scenePassDefinition);
	
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
	
	auto clipBuffer = core.createBuffer<float>(vkcv::BufferType::UNIFORM, 4);
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
	
	const vkcv::GraphicsPipelineConfig scenePipelineDefinition{
			sceneShaderProgram,
			UINT32_MAX,
			UINT32_MAX,
			scenePass,
			{sceneLayout},
			{ material0.getDescriptorSetLayout(), clipDescriptorSetLayout },
			true
	};
	
	const vkcv::GraphicsPipelineConfig linePipelineDefinition{
			lineShaderProgram,
			UINT32_MAX,
			UINT32_MAX,
			linePass,
			{sceneLayout},
			{ material0.getDescriptorSetLayout(), clipDescriptorSetLayout },
			true
	};
	
	vkcv::GraphicsPipelineHandle scenePipeline = core.createGraphicsPipeline(scenePipelineDefinition);
	vkcv::GraphicsPipelineHandle linePipeline = core.createGraphicsPipeline(linePipelineDefinition);
	
	if ((!scenePipeline) || (!linePipeline)) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	auto swapchainExtent = core.getSwapchain(windowHandle).getExtent();
	
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
			
			colorBuffer = core.createImage(
					colorFormat,
					swapchainExtent.width,
					swapchainExtent.height,
					1, false, true, true
			).getHandle();
			
			swapchainExtent.width = swapchainWidth;
			swapchainExtent.height = swapchainHeight;
		}
		
		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		
		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
		
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
		
		auto stop = std::chrono::system_clock::now();
		auto kektime = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		
		gui.beginGUI();
		
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Clip X", &clipX, -1.0f, 1.0f);
		ImGui::SliderFloat("Clip Y", &clipY, -1.0f, 1.0f);
		ImGui::SliderFloat("Clip Z", &clipZ, -1.0f, 1.0f);
		ImGui::Text("Mesh by HannahNewey (https://sketchfab.com/HannahNewey)");
		ImGui::End();
		
		gui.endGUI();
		
		core.endFrame(windowHandle);
	}
	
	return 0;
}
