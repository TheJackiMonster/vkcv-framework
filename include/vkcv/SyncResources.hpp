#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch
 * @file vkcv/SyncResources.hpp
 * @brief Support functions to deal with synchronization resources.
 */

#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	struct SyncResources {
		vk::Semaphore	renderFinished;
		vk::Semaphore	swapchainImageAcquired;
		vk::Fence		presentFinished;
	};

	SyncResources	createSyncResources(const vk::Device &device);
	void			destroySyncResources(const vk::Device &device, const SyncResources &resources);
	vk::Fence		createFence(const vk::Device &device);
	void			waitForFence(const vk::Device& device, const vk::Fence& fence);
	
}