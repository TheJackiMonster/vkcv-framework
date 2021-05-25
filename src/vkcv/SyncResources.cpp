#include "vkcv/SyncResources.hpp"

namespace vkcv {
	SyncResources createSyncResources(const vk::Device& device) {
		SyncResources resources;

		const vk::SemaphoreCreateFlags semaphoreFlags = vk::SemaphoreCreateFlagBits();
		const vk::SemaphoreCreateInfo semaphoreInfo(semaphoreFlags);
		resources.renderFinished			= device.createSemaphore(semaphoreInfo, nullptr, {});
		resources.swapchainImageAcquired	= device.createSemaphore(semaphoreInfo);

		resources.presentFinished			= createFence(device);
		
		return resources;
	}

	void destroySyncResources(const vk::Device& device, const SyncResources& resources) {
		device.destroySemaphore(resources.renderFinished);
		device.destroySemaphore(resources.swapchainImageAcquired);
		device.destroyFence(resources.presentFinished);
	}

	vk::Fence createFence(const vk::Device& device) {
		const vk::FenceCreateFlags fenceFlags = vk::FenceCreateFlagBits();
		vk::FenceCreateInfo fenceInfo(fenceFlags);
		return device.createFence(fenceInfo, nullptr, {});
	}

	void waitForFence(const vk::Device& device, const vk::Fence fence) {
		const auto result = device.waitForFences(fence, true, UINT64_MAX);
		assert(result == vk::Result::eSuccess);
	}
}