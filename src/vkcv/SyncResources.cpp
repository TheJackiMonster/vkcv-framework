#include "vkcv/SyncResources.hpp"

namespace vkcv {
	SyncResources createDefaultSyncResources(const vk::Device& device) {
		SyncResources resources;

		const vk::SemaphoreCreateFlags semaphoreFlags = vk::SemaphoreCreateFlagBits();
		const vk::SemaphoreCreateInfo semaphoreInfo(semaphoreFlags);
		resources.renderFinished = device.createSemaphore(semaphoreInfo, nullptr, {});

		const vk::FenceCreateFlags fenceFlags = vk::FenceCreateFlagBits();
		vk::FenceCreateInfo fenceInfo(fenceFlags);
		resources.presentFinished = device.createFence(fenceInfo, nullptr, {});
		resources.swapchainImageAcquired = device.createFence(fenceInfo, nullptr, {});

		return resources;
	}

	void destroySyncResources(const vk::Device& device, const SyncResources& resources) {
		device.destroySemaphore(resources.renderFinished);
		device.destroyFence(resources.presentFinished);
		device.destroyFence(resources.swapchainImageAcquired);
	}
}