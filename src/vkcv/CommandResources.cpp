#include "vkcv/CommandResources.hpp"
#include <iostream>

#include "vkcv/Logger.hpp"

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

	Queue getQueueForSubmit(const QueueType type, const QueueManager& queueManager) {
		if (type == QueueType::Graphics) {
			return queueManager.getGraphicsQueues().front();
		}
		else if (type == QueueType::Compute) {
			return queueManager.getComputeQueues().front();
		}
		else if (type == QueueType::Transfer) {
			return queueManager.getTransferQueues().front();
		}
		else if (type == QueueType::Present) {
			return queueManager.getPresentQueue();
		}
		else {
			vkcv_log(vkcv::LogLevel::ERROR, "Unknown queue type");
			return queueManager.getGraphicsQueues().front();	// graphics is the most general queue
		}
	}

	void beginCommandBuffer(const vk::CommandBuffer cmdBuffer, const vk::CommandBufferUsageFlags flags) {
		const vk::CommandBufferBeginInfo beginInfo(flags);
		cmdBuffer.begin(beginInfo);
	}

	void submitCommandBufferToQueue(
		const vk::Queue						queue,
		const vk::CommandBuffer				cmdBuffer,
		const vk::Fence						fence,
		const std::vector<vk::Semaphore>&	waitSemaphores,
		const std::vector<vk::Semaphore>&	signalSemaphores) {

		const std::vector<vk::PipelineStageFlags> waitDstStageMasks(waitSemaphores.size(), vk::PipelineStageFlagBits::eAllCommands);
		const vk::SubmitInfo queueSubmitInfo(waitSemaphores, waitDstStageMasks, cmdBuffer, signalSemaphores);
		queue.submit(queueSubmitInfo, fence);
	}
}