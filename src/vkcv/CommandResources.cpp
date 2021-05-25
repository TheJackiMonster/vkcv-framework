#include "vkcv/CommandResources.hpp"

namespace vkcv {

	std::unordered_set<int> generateQueueFamilyIndexSet(const QueueManager &queueManager) {
		std::unordered_set<int> indexSet;
		for (const auto& queue : queueManager.getGraphicsQueues()) {
			indexSet.insert(queue.familyIndex);
		}
		for (const auto& queue : queueManager.getComputeQueues()) {
			indexSet.insert(queue.familyIndex);
		}
		for (const auto& queue : queueManager.getTransferQueues()) {
			indexSet.insert(queue.familyIndex);
		}
		indexSet.insert(queueManager.getPresentQueue().familyIndex);
		return indexSet;
	}

	CommandResources createCommandResources(const vk::Device& device, const std::unordered_set<int>& familyIndexSet) {
		CommandResources resources;
		const size_t queueFamiliesCount = familyIndexSet.size();
		resources.cmdPoolPerQueueFamily.resize(queueFamiliesCount);

		const vk::CommandPoolCreateFlags poolFlags = vk::CommandPoolCreateFlagBits::eTransient;
		for (const int familyIndex : familyIndexSet) {
			const vk::CommandPoolCreateInfo poolCreateInfo(poolFlags, familyIndex);
			resources.cmdPoolPerQueueFamily[familyIndex] = device.createCommandPool(poolCreateInfo, nullptr, {});
		}

		return resources;
	}

	void destroyCommandResources(const vk::Device& device, const CommandResources& resources) {
		for (const vk::CommandPool &pool : resources.cmdPoolPerQueueFamily) {
			device.destroyCommandPool(pool);
		}
	}

	vk::CommandBuffer allocateCommandBuffer(const vk::Device& device, const vk::CommandPool cmdPool) {
		const vk::CommandBufferAllocateInfo info(cmdPool, vk::CommandBufferLevel::ePrimary, 1);
		return device.allocateCommandBuffers(info).front();
	}

	vk::CommandPool chooseCmdPool(const Queue& queue, const CommandResources& cmdResources) {
		return cmdResources.cmdPoolPerQueueFamily[queue.familyIndex];
	}
}