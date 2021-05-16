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

	std::vector<vk::DeviceQueueCreateInfo> createDeviceQueueCreateInfo(const QueueFamilyIndices& indices,
		std::vector<float>* outQueuePriorities) {

		// use set to avoid duplicate queues
		std::unordered_set<int> familyIndexSet;
		familyIndexSet.insert(indices.graphicsIndex);
		familyIndexSet.insert(indices.computeIndex);
		familyIndexSet.insert(indices.transferIndex);
		familyIndexSet.insert(indices.presentIndex);

		const vk::DeviceQueueCreateFlagBits flags = {};
		std::vector<vk::DeviceQueueCreateInfo> createInfos;

		outQueuePriorities->resize(familyIndexSet.size(), 1.f);
		int priorityIndex = 0;

		for (const auto index : familyIndexSet) {
			outQueuePriorities->push_back(1.f);
			const vk::DeviceQueueCreateInfo graphicsCreateInfo(flags, index, 1, &outQueuePriorities->at(priorityIndex));
			createInfos.push_back(graphicsCreateInfo);
			priorityIndex++;
		}
		return createInfos;
	}
}