#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch
 * @file vkcv/SyncResources.hpp
 * @brief Support functions to deal with synchronization resources.
 */

#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	/**
	 * @brief Structure to store vulkan resources for presenting
	 * with a pipeline.
	 */
	struct SyncResources {
		vk::Semaphore renderFinished;
		vk::Semaphore swapchainImageAcquired;
		vk::Fence presentFinished;
	};

	/**
	 * @brief Creates new synchronization resources for drawing
	 * and presenting with a swapchain.
	 *
	 * @param[in,out] device Vulkan-Device
	 * @return New created synchronization resources
	 */
	SyncResources createSyncResources(const vk::Device &device);
	
	/**
	 * @brief Destroys the synchronization resources with a
	 * given device.
	 *
	 * @param[in,out] device Vulkan-Device
	 * @param[in,out] resources Synchronizazion resources
	 */
	void destroySyncResources(const vk::Device &device,
							  const SyncResources &resources);
	
	/**
	 * @brief Creates a new fence with a given device and
	 * returns it.
	 *
	 * @param[in,out] device Vulkan-Device
	 * @return New created fence
	 */
	vk::Fence createFence(const vk::Device &device);
	
	/**
	 * @brief Calls a given device to wait for a specific fence.
	 *
	 * @param[in,out] device Vulkan-Device
	 * @param[in] fence Vulkan-Fence
	 */
	void waitForFence(const vk::Device& device,
					  const vk::Fence& fence);
	
}