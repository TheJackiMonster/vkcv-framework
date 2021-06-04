#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "First Mesh";

	const int windowWidth = 800;
	const int windowHeight = 600;
	vkcv::Window window = vkcv::Window::create(
		applicationName,
		windowWidth,
		windowHeight,
		true
	);

	vkcv::CameraManager cameraManager(window, windowWidth, windowHeight);
	cameraManager.getCamera().setPosition(glm::vec3(0.f, 0.f, 3.f));

	window.initEvents();

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
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

	assert(mesh.vertexGroups.size() > 0);
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

	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
		vkcv::VertexBufferBinding(mesh.vertexGroups[0].vertexBuffer.attributes[0].offset, vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(mesh.vertexGroups[0].vertexBuffer.attributes[1].offset, vertexBuffer.getVulkanHandle()),
		vkcv::VertexBufferBinding(mesh.vertexGroups[0].vertexBuffer.attributes[2].offset, vertexBuffer.getVulkanHandle()) };

	const vkcv::Mesh loadedMesh(vertexBufferBindings, indexBuffer.getVulkanHandle(), mesh.vertexGroups[0].numIndices);

	// an example attachment for passes that output to the window
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentLayout::UNDEFINED,
		vkcv::AttachmentLayout::COLOR_ATTACHMENT,
		vkcv::AttachmentLayout::PRESENTATION,
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		core.getSwapchainImageFormat()
	);
	
	const vkcv::AttachmentDescription depth_attachment(
			vkcv::AttachmentLayout::UNDEFINED,
			vkcv::AttachmentLayout::DEPTH_STENCIL_ATTACHMENT,
			vkcv::AttachmentLayout::DEPTH_STENCIL_ATTACHMENT,
			vkcv::AttachmentOperation::STORE,
			vkcv::AttachmentOperation::CLEAR,
			vk::Format::eD32Sfloat
	);

	vkcv::PassConfig trianglePassDefinition({ present_color_attachment, depth_attachment });
	vkcv::PassHandle trianglePass = core.createPass(trianglePassDefinition);

	if (!trianglePass) {
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram triangleShaderProgram{};
	triangleShaderProgram.addShader(vkcv::ShaderStage::VERTEX, std::filesystem::path("resources/shaders/vert.spv"));
	triangleShaderProgram.addShader(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("resources/shaders/frag.spv"));
	triangleShaderProgram.reflectShader(vkcv::ShaderStage::VERTEX);
	triangleShaderProgram.reflectShader(vkcv::ShaderStage::FRAGMENT);
	
	auto& attributes = mesh.vertexGroups[0].vertexBuffer.attributes;
	
	std::sort(attributes.begin(), attributes.end(), [](const vkcv::VertexAttribute& x, const vkcv::VertexAttribute& y) {
		return static_cast<uint32_t>(x.type) < static_cast<uint32_t>(y.type);
	});

	std::vector<vkcv::DescriptorBinding> descriptorBindings = {
		vkcv::DescriptorBinding(vkcv::DescriptorType::IMAGE_SAMPLED,	1, vkcv::ShaderStage::FRAGMENT),
		vkcv::DescriptorBinding(vkcv::DescriptorType::SAMPLER,			1, vkcv::ShaderStage::FRAGMENT)};
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorBindings);

	const vkcv::PipelineConfig trianglePipelineDefinition(
		triangleShaderProgram, 
		windowWidth,
		windowHeight,
		trianglePass,
		mesh.vertexGroups[0].vertexBuffer.attributes,
		{ core.getDescriptorSet(descriptorSet).layout },
		true);
	vkcv::PipelineHandle trianglePipeline = core.createGraphicsPipeline(trianglePipelineDefinition);
	
	if (!trianglePipeline) {
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

	vkcv::DescriptorWrites setWrites;
	setWrites.sampledImageWrites	= { vkcv::SampledImageDescriptorWrite(0, texture.getHandle()) };
	setWrites.samplerWrites			= { vkcv::SamplerDescriptorWrite(1, sampler) };
	core.writeResourceDescription(descriptorSet, 0, setWrites);

	vkcv::ImageHandle depthBuffer = core.createImage(vk::Format::eD32Sfloat, windowWidth, windowHeight).getHandle();

	window.e_resize.add([&](int width, int height) {
		depthBuffer = core.createImage(vk::Format::eD32Sfloat, width, height).getHandle();
	});

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };

	const vkcv::DescriptorSetUsage descriptorUsage(0, core.getDescriptorSet(descriptorSet).vulkanHandle);

	const std::vector<glm::vec3> instancePositions = {
		glm::vec3( 0.f, -2.f, 0.f),
		glm::vec3( 3.f,  0.f, 0.f),
		glm::vec3(-3.f,  0.f, 0.f),
		glm::vec3( 0.f,  2.f, 0.f)
	};

	std::vector<glm::mat4> modelMatrices;
	std::vector<vkcv::DrawcallInfo> drawcalls;
	for (const auto& position : instancePositions) {
		modelMatrices.push_back(glm::translate(glm::mat4(1.f), position));
		drawcalls.push_back(vkcv::DrawcallInfo(loadedMesh, { descriptorUsage }));
	}

	std::vector<glm::mat4> mvpMatrices;

	auto start = std::chrono::system_clock::now();
	while (window.isWindowOpen()) {
		window.pollEvents();
		core.beginFrame();
		auto end = std::chrono::system_clock::now();
		auto deltatime = end - start;
		start = end;
		cameraManager.getCamera().updateView(std::chrono::duration<double>(deltatime).count());

		const glm::mat4 viewProjection = cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView();
		mvpMatrices.clear();
		for (const auto& m : modelMatrices) {
			mvpMatrices.push_back(viewProjection * m);
		}
		vkcv::PushConstantData pushConstantData((void*)mvpMatrices.data(), sizeof(glm::mat4));

		core.recordDrawcalls(
			trianglePass,
			trianglePipeline,
			pushConstantData,
			drawcalls,
			renderTargets);

		core.endFrame();
	}
	
	return 0;
}
