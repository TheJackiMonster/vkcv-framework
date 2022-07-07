#include <iostream>
#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "First Mesh";

	vkcv::Core core = vkcv::Core::create(
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }
	);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 800, 600, true);

	vkcv::asset::Scene mesh;

	const char* path = argc > 1 ? argv[1] : "assets/cube/cube.gltf";
	int result = vkcv::asset::loadScene(path, mesh);

	if (result == 1) {
		std::cout << "Mesh loading successful!" << std::endl;
	} else {
		std::cerr << "Mesh loading failed: " << result << std::endl;
		return 1;
	}

	assert(!mesh.vertexGroups.empty());
	auto vertexBuffer = vkcv::buffer<uint8_t>(
			core,
			vkcv::BufferType::VERTEX,
			mesh.vertexGroups[0].vertexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	vertexBuffer.fill(mesh.vertexGroups[0].vertexBuffer.data);

	auto indexBuffer = vkcv::buffer<uint8_t>(
			core,
			vkcv::BufferType::INDEX,
			mesh.vertexGroups[0].indexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	indexBuffer.fill(mesh.vertexGroups[0].indexBuffer.data);

	// an example attachment for passes that output to the window
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

	vkcv::PassConfig firstMeshPassDefinition(
			{ present_color_attachment, depth_attachment },
			vkcv::Multisampling::None
	);

	vkcv::PassHandle firstMeshPass = core.createPass(firstMeshPassDefinition);

	if (!firstMeshPass) {
		std::cerr << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram firstMeshProgram;
	vkcv::shader::GLSLCompiler compiler;
	
	compiler.compileProgram(firstMeshProgram, {
		{ vkcv::ShaderStage::VERTEX, "assets/shaders/shader.vert" },
		{ vkcv::ShaderStage::FRAGMENT, "assets/shaders/shader.frag" }
	}, nullptr);
 
	auto& attributes = mesh.vertexGroups[0].vertexBuffer.attributes;
	
	std::sort(attributes.begin(), attributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
		return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
	});

    const std::vector<vkcv::VertexAttachment> vertexAttachments = firstMeshProgram.getVertexAttachments();
	std::vector<vkcv::VertexBinding> bindings;
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::createVertexBinding(i, { vertexAttachments[i] }));
	}
	
	const vkcv::VertexLayout firstMeshLayout { bindings };

	// since we only use one descriptor set (namely, desc set 0), directly address it
	// recreate copies of the bindings and the handles (to check whether they are properly reused instead of actually recreated)
	const vkcv::DescriptorBindings& set0Bindings = firstMeshProgram.getReflectedDescriptors().at(0);
    auto set0BindingsExplicitCopy = set0Bindings;

	vkcv::DescriptorSetLayoutHandle setLayoutHandle = core.createDescriptorSetLayout(set0Bindings);
	vkcv::DescriptorSetLayoutHandle setLayoutHandleCopy = core.createDescriptorSetLayout(set0BindingsExplicitCopy);

	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(setLayoutHandle);
	
	vkcv::GraphicsPipelineHandle firstMeshPipeline = core.createGraphicsPipeline(
			vkcv::GraphicsPipelineConfig(
					firstMeshProgram,
					firstMeshPass,
					{ firstMeshLayout },
					{ setLayoutHandle }
			)
	);
	
	if (!firstMeshPipeline) {
		std::cerr << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	if (mesh.textures.empty()) {
		std::cerr << "Error. No textures found. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	vkcv::asset::Texture &tex = mesh.textures[0];
	vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Srgb, tex.w, tex.h);
	texture.fill(tex.data.data());
	
	{
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		texture.recordMipChainGeneration(cmdStream, core.getDownsampler());
		core.submitCommandStream(cmdStream, false);
	}

	vkcv::SamplerHandle sampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::REPEAT
	);

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[0].offset), vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[1].offset), vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(static_cast<vk::DeviceSize>(attributes[2].offset), vertexBuffer.getVulkanHandle()) };

	vkcv::DescriptorWrites setWrites;
	setWrites.writeSampledImage(0, texture.getHandle());
	setWrites.writeSampler(1, sampler);

	core.writeDescriptorSet(descriptorSet, setWrites);
	
	vkcv::ImageHandle depthBuffer;

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	const vkcv::Mesh renderMesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), mesh.vertexGroups[0].numIndices);

	vkcv::DescriptorSetUsage    descriptorUsage(0, descriptorSet);
	vkcv::DrawcallInfo          drawcall(renderMesh, { descriptorUsage },1);

    vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
    uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -3));

    auto start = std::chrono::system_clock::now();
    
	while (vkcv::Window::hasOpenWindow()) {
        vkcv::Window::pollEvents();
		
		if(core.getWindow(windowHandle).getHeight() == 0 || core.getWindow(windowHandle).getWidth() == 0)
			continue;
		
		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight, windowHandle)) {
			continue;
		}
		
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			(swapchainHeight != core.getImageHeight(depthBuffer))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					swapchainWidth,
					swapchainHeight
			).getHandle();
		}
  
		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		
		start = end;
		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));
        glm::mat4 mvp = cameraManager.getActiveCamera().getMVP();

		vkcv::PushConstants pushConstants = vkcv::pushConstants<glm::mat4>();
		pushConstants.appendDrawcall(mvp);

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordDrawcallsToCmdStream(
			cmdStream,
			firstMeshPass,
			firstMeshPipeline,
			pushConstants,
			{ drawcall },
			renderTargets,
			windowHandle);
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		core.endFrame(windowHandle);
	}
	
	return 0;
}
