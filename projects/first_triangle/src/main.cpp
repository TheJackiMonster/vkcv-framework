#include <iostream>
#include <vkcv/Core.hpp>
#include <vkcv/Window.hpp>

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
	
	struct vec3 {
		float x, y, z;
	};
	
	auto buffer = core.createBuffer<vec3>(vkcv::BufferType::VERTEX, 3);
	
	vec3* m = buffer.map();
	m[0] = { 0, 0, 0 };
	m[0] = { 0, 0, 0 };
	m[0] = { 0, 0, 0 };

	std::cout << "Physical device: " << physicalDevice.getProperties().deviceName << std::endl;

	switch (physicalDevice.getProperties().vendorID) {
		case 0x1002: std::cout << "Running AMD huh? You like underdogs, are you a Linux user?" << std::endl; break;
		case 0x10DE: std::cout << "An NVidia GPU, how predictable..." << std::endl; break;
		case 0x8086: std::cout << "Poor child, running on an Intel GPU, probably integrated..."
			"or perhaps you are from the future with a dedicated one?" << std::endl; break;
		case 0x13B5: std::cout << "ARM? What the hell are you running on, next thing I know you're trying to run Vulkan on a leg..." << std::endl; break;
		default: std::cout << "Unknown GPU vendor?! Either you're on an exotic system or your driver is broken..." << std::endl;
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
