#pragma once
#include <vulkan/vulkan.hpp>
#include <unordered_set>
#include "QueueManager.hpp"

namespace vkcv {
	struct CommandResources {
		std::vector<vk::CommandPool> cmdPoolPerQueueFamily;
	};

	std::unordered_set<int> generateQueueFamilyIndexSet(const QueueManager& queueManager);
	CommandResources		createCommandResources(const vk::Device& device, const std::unordered_set<int> &familyIndexSet);
	void					destroyCommandResources(const vk::Device& device, const CommandResources& resources);
	vk::CommandBuffer		allocateCommandBuffer(const vk::Device& device, const vk::CommandPool cmdPool);
	vk::CommandPool			chooseCmdPool(const Queue &queue, const CommandResources &cmdResources);
	Queue					getQueueForSubmit(const QueueType type, const QueueManager &queueManager);
	void					beginCommandBuffer(const vk::CommandBuffer cmdBuffer, const vk::CommandBufferUsageFlags flags);

	void submitCommandBufferToQueue(
		const vk::Queue						queue,
		const vk::CommandBuffer				cmdBuffer,
		const vk::Fence						fence,
		const std::vector<vk::Semaphore>&	waitSemaphores,
		const std::vector<vk::Semaphore>&	signalSemaphores);
}