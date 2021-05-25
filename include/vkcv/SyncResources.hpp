#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv {
	struct SyncResources {
		vk::Semaphore	renderFinished;
		vk::Fence		swapchainImageAcquired;
		vk::Fence		presentFinished;
	};

	SyncResources	createSyncResources(const vk::Device &device);
	void			destroySyncResources(const vk::Device &device, const SyncResources &resources);
	vk::Fence		createFence(const vk::Device &device);
	void			waitForFence(const vk::Device& device, const vk::Fence fence);
}