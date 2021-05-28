#include "Surface.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vkcv {
	/**
	* creates surface and checks availability
	* @param window current window for the surface
	* @param instance Vulkan-Instance
	* @param physicalDevice Vulkan-PhysicalDevice
	* @return created surface
	*/
	vk::SurfaceKHR createSurface(GLFWwindow* window, const vk::Instance& instance, const vk::PhysicalDevice& physicalDevice) {
		//create surface
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create a window surface!");
		}
		vk::Bool32 surfaceSupport = false;
		if (physicalDevice.getSurfaceSupportKHR(0, vk::SurfaceKHR(surface), &surfaceSupport) != vk::Result::eSuccess && surfaceSupport != true) {
			throw std::runtime_error("surface is not supported by the device!");
		}

		return vk::SurfaceKHR(surface);
	}
}
