#include <iostream>
#include <vkcv/Context.hpp>
#include <vkcv/Window.hpp>
#include <vkcv/SwapChain.hpp>

int main(int argc, const char** argv) {
    const char* applicationName = "First Triangle";
	vkcv::Window window = vkcv::Window::create(
            applicationName,
        800,
        600,
		false
	);
	vkcv::Context context = vkcv::Context::create(
            applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		20,
		{vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eTransfer},
		{},
		{"VK_KHR_swapchain"}
	);

	GLFWwindow *glWindow = window.getWindow();
	const vk::Instance& instance = context.getInstance();
	const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
	const vk::Device& device = context.getDevice();
    const vkcv::SwapChain& swapChain = vkcv::SwapChain::create(glWindow, instance, physicalDevice, device);

	std::cout << "Physical device: " << physicalDevice.getProperties().deviceName << std::endl;

	switch (physicalDevice.getProperties().vendorID) {
		case 0x1002: std::cout << "Running AMD huh? You like underdogs, are you a Linux user?" << std::endl; break;
		case 0x10DE: std::cout << "An NVidia GPU, how predictable..." << std::endl; break;
		case 0x8086: std::cout << "Poor child, running on an Intel GPU, probably integrated..."
			"or perhaps you are from the future with a dedicated one?" << std::endl; break;
		case 0x13B5: std::cout << "ARM? What the hell are you running on, next thing I know you're trying to run Vulkan on a leg..." << std::endl; break;
		default: std::cout << "Unknown GPU vendor?! Either you're on an exotic system or your driver is broken..." << std::endl;
	}

	while (window.isWindowOpen()) {
		window.pollEvents();
	}
	return 0;
}
