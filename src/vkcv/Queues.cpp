#include "vkcv/Queues.hpp"
#include <unordered_set>

namespace vkcv {

	VulkanQueues getDeviceQueues(const vk::Device& device, const QueueFamilyIndices& familyIndices) {
		VulkanQueues queues;
		queues.graphicsQueue = device.getQueue(familyIndices.graphicsIndex, 0);
		queues.computeQueue = device.getQueue(familyIndices.computeIndex, 0);
		queues.transferQueue = device.getQueue(familyIndices.transferIndex, 0);
		queues.presentQueue = device.getQueue(familyIndices.presentIndex, 0);
		return queues;
	}

	QueueFamilyIndices getQueueFamilyIndices(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR surface) {
		const std::vector<vk::QueueFamilyProperties> familyProps = physicalDevice.getQueueFamilyProperties();

		QueueFamilyIndices indices;

		for (int i = 0; i < familyProps.size(); i++) {
			const auto& property = familyProps[i];

			const bool hasQueues = property.queueCount >= 1;
			if (!hasQueues) {
				continue;
			}

			if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
				indices.graphicsIndex = i;
			}
			if (property.queueFlags & vk::QueueFlagBits::eCompute) {
				indices.computeIndex = i;
			}
			if (property.queueFlags & vk::QueueFlagBits::eTransfer) {
				indices.transferIndex = i;
			}
			if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
				indices.presentIndex = i;
			}
		}
		assert(indices.graphicsIndex != -1);
		assert(indices.computeIndex  != -1);
		assert(indices.transferIndex != -1);
		assert(indices.presentIndex  != -1);
		return indices;
	}

	std::vector<vk::DeviceQueueCreateInfo> createDeviceQueueCreateInfo(const QueueFamilyIndices& indices) {

		// use set to avoid duplicate queues
		std::unordered_set<int> familyIndexSet;
		familyIndexSet.insert(indices.graphicsIndex);
		familyIndexSet.insert(indices.computeIndex);
		familyIndexSet.insert(indices.transferIndex);
		familyIndexSet.insert(indices.presentIndex);

		const vk::DeviceQueueCreateFlagBits flags = {};
		const float priority = 1.f;
		std::vector<vk::DeviceQueueCreateInfo> createInfos;

		for (const auto index : familyIndexSet) {
			const vk::DeviceQueueCreateInfo graphicsCreateInfo(flags, index, 1, &priority);
			createInfos.push_back(graphicsCreateInfo);
		}
		return createInfos;
	}
}