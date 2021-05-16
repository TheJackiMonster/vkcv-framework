#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Window.hpp>
#include <vkcv/ShaderProgram.hpp>

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
            {vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
		{},
		{"VK_KHR_swapchain"}
	);

	const auto &context = core.getContext();
	const vk::Instance& instance = context.getInstance();
	const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
	const vk::Device& device = context.getDevice();

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
		vkcv::AttachmentLayout::UNDEFINED,
		vkcv::AttachmentLayout::COLOR_ATTACHMENT,
		vkcv::AttachmentLayout::PRESENTATION,
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR);

	vkcv::PassConfig trianglePassDefinition({present_color_attachment});
	vkcv::PassHandle trianglePass = core.createPass(trianglePassDefinition);

	if (trianglePass.id == 0)
	{
		std::cout << "Error. Could not create renderpass. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

	vkcv::ShaderProgram triangleShaderProgram{};
	triangleShaderProgram.addShader(vkcv::ShaderStage::VERTEX, std::filesystem::path("shaders/vert.spv"));
	triangleShaderProgram.addShader(vkcv::ShaderStage::FRAGMENT, std::filesystem::path("shaders/frag.spv"));

	const vkcv::PipelineConfig trianglePipelineDefinition(triangleShaderProgram, windowWidth, windowHeight, trianglePass);
	vkcv::PipelineHandle trianglePipeline = core.createGraphicsPipeline(trianglePipelineDefinition);
	if (trianglePipeline.id == 0)
	{
		std::cout << "Error. Could not create graphics pipeline. Exiting." << std::endl;
		return EXIT_FAILURE;
	}

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

	while (window.isWindowOpen())
	{
        // core.beginFrame(); or something like that
	    // core.execute(trianglePass, trianglePipeline, triangleModel);
	    // core.endFrame(); or something like that

	    // TBD: synchronization

		window.pollEvents();
	}
	return 0;
}
