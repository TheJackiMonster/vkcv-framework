#pragma once
#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace vkcv {	
	vk::SurfaceKHR createSurface(GLFWwindow* window, const vk::Instance& instance, const vk::PhysicalDevice& physicalDevice);
}