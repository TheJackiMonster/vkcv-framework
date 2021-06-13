#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "First Mesh";

	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;

	vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		true
	);

	vkcv::CameraManager cameraManager(window, static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));

	window.initEvents();

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
		{},
		{ "VK_KHR_swapchain" }
	);

	vkcv::asset::Mesh mesh;

	const char* path = argc > 1 ? argv[1] : "resources/cube/cube.gltf";
	int result = vkcv::asset::loadMesh(path, mesh);

	if (result == 1) {
		std::cout << "Mesh loading successful!" << std::endl;
	}
	else {
		std::cout << "Mesh loading failed: " << result << std::endl;
		return 1;
	}

	assert(!mesh.vertexGroups.empty());
	auto vertexBuffer = core.createBuffer<uint8_t>(
			vkcv::BufferType::VERTEX,
			mesh.vertexGroups[0].vertexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	vertexBuffer.fill(mesh.vertexGroups[0].vertexBuffer.data);

	auto indexBuffer = core.createBuffer<uint8_t>(
			vkcv::BufferType::INDEX,
			mesh.vertexGroups[0].indexBuffer.data.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL
	);
	
	indexBuffer.fill(mesh.vertexGroups[0].indexBuffer.data);

	// an example attachment for passes that output to the window
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchainImageFormat()
	);
	
	const vkcv::AttachmentDescription depth_attachment(
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::CLEAR,
			vk::Format::eD32Sfloat
	);

	vkcv::PassConfig firstMeshPassDefinition({ present_color_attachment, depth_attachment });
	vkcv::PassHandle firstMeshPass = core.createPass(firstMeshPassDefinition);

	if (!firstMeshPass) {
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram firstMeshProgram{};
    firstMeshProgram.addShader(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/vert.spv"));
    firstMeshProgram.addShader(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/frag.spv"));

	/**
	 * TODO:
	 *  Since the framework's vertex layout specification is now separate
	 *  from that defined in the asset loader module, there needs to be a smarter way to translate the asset loader's
	 *  specific layout into "our" uniform vertex layout spec.
	 *
	 */

    const std::vector<vkcv::VertexAttachment> vertexAttachments = firstMeshProgram.getVertexAttachments();

    const vkcv::VertexBinding vertexBinding(0, vertexAttachments);
    const vkcv::VertexLayout firstMeshLayout({vertexBinding});

	uint32_t setID = 0;
	std::vector<vkcv::DescriptorBinding> descriptorBindings = { firstMeshProgram.getReflectedDescriptors()[setID] };
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

	const vkcv::PipelineConfig firstMeshPipelineConfig(
        firstMeshProgram,
        UINT32_MAX,
        UINT32_MAX,
        firstMeshPass,
        {firstMeshLayout},
		{ core.getDescriptorSet(descriptorSet).layout },
		true);
	vkcv::PipelineHandle firstMeshPipeline = core.createGraphicsPipeline(firstMeshPipelineConfig);
	
	if (!firstMeshPipeline) {
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}
	
	vkcv::Image texture = core.createImage(vk::Format::eR8G8B8A8Srgb, mesh.texture_hack.w, mesh.texture_hack.h);
	texture.fill(mesh.texture_hack.img);

	vkcv::SamplerHandle sampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::REPEAT
	);

    auto& attributes = mesh.vertexGroups[0].vertexBuffer.attributes;

    std::sort(attributes.begin(), attributes.end(), [](const vkcv::asset::VertexAttribute& x, const vkcv::asset::VertexAttribute& y) {
        return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
    });

    /*
	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
		vkcv::VertexBufferBinding( static_cast<vk::DeviceSize>(mesh.vertexGroups[0].vertexBuffer.attributes[0].offset), vertexBuffer.getVulkanHandle() ),
		vkcv::VertexBufferBinding( static_cast<vk::DeviceSize>(mesh.vertexGroups[0].vertexBuffer.attributes[1].offset), vertexBuffer.getVulkanHandle() ),
		vkcv::VertexBufferBinding( static_cast<vk::DeviceSize>(mesh.vertexGroups[0].vertexBuffer.attributes[2].offset), vertexBuffer.getVulkanHandle() )
	};
    */
    const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
            vkcv::VertexBufferBinding( 0, vertexBuffer.getVulkanHandle() ),
    };

	vkcv::DescriptorWrites setWrites;
	setWrites.sampledImageWrites    = { vkcv::SampledImageDescriptorWrite(0, texture.getHandle()) };
	setWrites.samplerWrites         = { vkcv::SamplerDescriptorWrite(1, sampler) };
	core.writeDescriptorSet(descriptorSet, setWrites);

	vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight).getHandle();

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	const vkcv::Mesh renderMesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), mesh.vertexGroups[0].numIndices);

	vkcv::DescriptorSetUsage    descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);
	vkcv::DrawcallInfo          drawcall(renderMesh, { descriptorUsage });

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
		auto deltatime = end - start;
		start = end;
		cameraManager.getCamera().updateView(std::chrono::duration<double>(deltatime).count());
		const glm::mat4 mvp = cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView();

		vkcv::PushConstantData pushConstantData((void*)&mvp, sizeof(glm::mat4));

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.recordDrawcallsToCmdStream(
			cmdStream,
			firstMeshPass,
			firstMeshPipeline,
			pushConstantData,
			{ drawcall },
			renderTargets);
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		core.endFrame();
	}
	
	return 0;
}
