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
	
	vkcv::gui::GUI gui (core, window);

	const auto& context = core.getContext();
	const vk::Instance& instance = context.getInstance();
	const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
	const vk::Device& device = context.getDevice();

	struct vec3 {
		float x, y, z;
	};

	const size_t n = 5027;

	auto testBuffer = core.createBuffer<vec3>(vkcv::BufferType::VERTEX, n, vkcv::BufferMemoryType::DEVICE_LOCAL);
	vec3 vec_data[n];

	for (size_t i = 0; i < n; i++) {
		vec_data[i] = { 42, static_cast<float>(i), 7 };
	}

	testBuffer.fill(vec_data);

	auto triangleIndexBuffer = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, n, vkcv::BufferMemoryType::DEVICE_LOCAL);
	uint16_t indices[3] = { 0, 1, 2 };
	triangleIndexBuffer.fill(&indices[0], sizeof(indices));

	/*vec3* m = buffer.map();
	m[0] = { 0, 0, 0 };
	m[1] = { 0, 0, 0 };
	m[2] = { 0, 0, 0 };
	buffer.unmap();*/

	vkcv::SamplerHandle sampler = core.createSampler(
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerFilterType::NEAREST,
		vkcv::SamplerMipmapMode::NEAREST,
		vkcv::SamplerAddressMode::REPEAT
	);

	std::cout << "Physical device: " << physicalDevice.getProperties().deviceName << std::endl;

	switch (physicalDevice.getProperties().vendorID) {
	case 0x1002: std::cout << "Running AMD huh? You like underdogs, are you a Linux user?" << std::endl; break;
	case 0x10DE: std::cout << "An NVidia GPU, how predictable..." << std::endl; break;
	case 0x8086: std::cout << "Poor child, running on an Intel GPU, probably integrated..."
		"or perhaps you are from the future with a dedicated one?" << std::endl; break;
	case 0x13B5: std::cout << "ARM? What the hell are you running on, next thing I know you're trying to run Vulkan on a leg..." << std::endl; break;
	default: std::cout << "Unknown GPU vendor?! Either you're on an exotic system or your driver is broken..." << std::endl;
	}

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

	// Compute Pipeline
	vkcv::ShaderProgram computeShaderProgram{};
	computeShaderProgram.addShader(vkcv::ShaderStage::COMPUTE, std::filesystem::path("shaders/comp.spv"));

	// take care, assuming shader has exactly one descriptor set
	vkcv::DescriptorSetHandle computeDescriptorSet = core.createDescriptorSet(computeShaderProgram.getReflectedDescriptors()[0]);

	vkcv::PipelineHandle computePipeline = core.createComputePipeline(
		computeShaderProgram, 
		{ core.getDescriptorSet(computeDescriptorSet).layout });

	struct ComputeTestBuffer {
		float test1[10];
		float test2[10];
		float test3[10];
	};

	vkcv::Buffer computeTestBuffer = core.createBuffer<ComputeTestBuffer>(vkcv::BufferType::STORAGE, 1);

	vkcv::DescriptorWrites computeDescriptorWrites;
	computeDescriptorWrites.storageBufferWrites = { vkcv::StorageBufferDescriptorWrite(0, computeTestBuffer.getHandle()) };
	core.writeDescriptorSet(computeDescriptorSet, computeDescriptorWrites);

	/*
	 * BufferHandle triangleVertices = core.createBuffer(vertices);
	 * BufferHandle triangleIndices = core.createBuffer(indices);
	 *
	 * // triangle Model creation goes here
	 *
	 *
	 * // attachment creation goes here
	 * PassHandle trianglePass = core.CreatePass(presentationPass);
	 *
	 * // shader creation goes here
	 * // material creation goes here
	 *
	 * PipelineHandle trianglePipeline = core.CreatePipeline(trianglePipeline);
	 */
	auto start = std::chrono::system_clock::now();

	vkcv::ImageHandle swapchainImageHandle = vkcv::ImageHandle::createSwapchainImageHandle();

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

		core.recordDrawcallsToCmdStream(
			cmdStream,
			trianglePass,
			trianglePipeline,
			pushConstantData,
			{ drawcall },
			{ swapchainInput });

		const uint32_t dispatchSize[3] = { 2, 1, 1 };
		const float theMeaningOfLife = 42;

		core.recordComputeDispatchToCmdStream(
			cmdStream,
			computePipeline,
			dispatchSize,
			{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(computeDescriptorSet).vulkanHandle) },
			vkcv::PushConstantData((void*)&theMeaningOfLife, sizeof(theMeaningOfLife)));

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		
		ImGui::Begin("Hello world");
		ImGui::Text("This is a test!");
		ImGui::End();
		
		gui.endGUI();
	    
	    core.endFrame();
	}
	return 0;
}
