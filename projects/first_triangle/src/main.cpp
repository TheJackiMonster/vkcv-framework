#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Window.hpp>
#include <vkcv/ShaderProgram.hpp>

int main(int argc, const char** argv) {
    const char* applicationName = "First Triangle";
	vkcv::Window window = vkcv::Window::create(
            applicationName,
        800,
        600,
		false
	);
	vkcv::Core core = vkcv::Core::create(
            applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		20,
		{vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eTransfer}
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

<<<<<<< HEAD
	vkcv::ShaderProgram shaderProgram = vkcv::ShaderProgram::create(context);
	shaderProgram.addShader(vk::ShaderStageFlagBits::eVertex , "../../../../../shaders/vert.spv");
	shaderProgram.addShader(vk::ShaderStageFlagBits::eFragment, "../../../../../shaders/frag.spv");

	while (window.isWindowOpen()) {
=======
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

>>>>>>> 74ff8e7041d7958006a434cf4d3a7fee0a3fa724
		window.pollEvents();
	}
	return 0;
}
