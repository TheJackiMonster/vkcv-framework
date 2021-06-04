#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/asset/asset_loader.hpp>

int main(int argc, const char** argv) {
	const char* applicationName = "First Mesh";

	vkcv::Window window = vkcv::Window::create(
		applicationName,
        800,
        600,
		true
	);

	vkcv::CameraManager cameraManager(window, static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));

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

	vkcv::DescriptorSetConfig setConfig({
		vkcv::DescriptorBinding(vkcv::DescriptorType::IMAGE_SAMPLED,	1, vkcv::ShaderStage::FRAGMENT),
		vkcv::DescriptorBinding(vkcv::DescriptorType::SAMPLER,			1, vkcv::ShaderStage::FRAGMENT)
	});
	vkcv::ResourcesHandle set = core.createResourceDescription({ setConfig });

	//only exemplary code for testing
	for (int i = 0; i < 1001; i++) {
		vkcv::ResourcesHandle furtherSets = core.createResourceDescription({ setConfig });
	}
	//end of exemplary code

	const vkcv::PipelineConfig trianglePipelineDefinition(
		triangleShaderProgram,
        UINT32_MAX,
        UINT32_MAX,
		trianglePass,
		mesh.vertexGroups[0].vertexBuffer.attributes,
		{ core.getDescriptorSetLayout(set, 0) });
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

	std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
		{ mesh.vertexGroups[0].vertexBuffer.attributes[0].offset, vertexBuffer.getHandle() },
		{ mesh.vertexGroups[0].vertexBuffer.attributes[1].offset, vertexBuffer.getHandle() },
		{ mesh.vertexGroups[0].vertexBuffer.attributes[2].offset, vertexBuffer.getHandle() }
	};

	vkcv::DescriptorWrites setWrites;
	setWrites.sampledImageWrites	= { vkcv::SampledImageDescriptorWrite(0, texture.getHandle()) };
	setWrites.samplerWrites			= { vkcv::SamplerDescriptorWrite(1, sampler) };
	core.writeResourceDescription(set, 0, setWrites);

	auto start = std::chrono::system_clock::now();
	while (window.isWindowOpen()) {
        vkcv::Window::pollEvents();

        if(window.getHeight() == 0 || window.getWidth() == 0)
            continue;

		core.beginFrame();
		auto end = std::chrono::system_clock::now();
		auto deltatime = end - start;
		start = end;
		cameraManager.getCamera().updateView(std::chrono::duration<double>(deltatime).count());
		const glm::mat4 mvp = cameraManager.getCamera().getProjection() * cameraManager.getCamera().getView();

		core.renderMesh(
			trianglePass,
			trianglePipeline,
			sizeof(mvp),
			&mvp,
			vertexBufferBindings,
			indexBuffer.getHandle(),
			mesh.vertexGroups[0].numIndices,
			set,
			0
		);

		core.endFrame();
	}
	
	return 0;
}
