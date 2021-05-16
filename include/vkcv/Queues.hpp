#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv {
	struct VulkanQueues {
		vk::Queue graphicsQueue;
		vk::Queue computeQueue;
		vk::Queue transferQueue;
		vk::Queue presentQueue;
	};

	struct QueueFamilyIndices {
		int graphicsIndex = -1;
		int computeIndex = -1;
		int transferIndex = -1;
		int presentIndex = -1;
	};

	VulkanQueues getDeviceQueues(const vk::Device& device, const QueueFamilyIndices& familyIndices);

	QueueFamilyIndices getQueueFamilyIndices(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR surface);

	// TODO: try to use specialised queues
	std::vector<vk::DeviceQueueCreateInfo> createDeviceQueueCreateInfo(const QueueFamilyIndices& indices);

}
