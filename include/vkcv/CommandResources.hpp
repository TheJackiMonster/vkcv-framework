#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch
 * @file vkcv/CommandResources.hpp
 * @brief Support functions to deal with command resources.
 */

#include <vulkan/vulkan.hpp>
#include <unordered_set>

#include "QueueManager.hpp"

namespace vkcv {
	
	/**
	 * @brief Structure to store command pools for given queue families
	 * of a device.
	 */
	struct CommandResources {
		std::vector<vk::CommandPool> cmdPoolPerQueueFamily;
	};

	/**
	 * @brief Generates a set of the family indices for all different kinds of
	 * queues a given queue manager provides.
	 *
	 * @param[in] queueManager Queue manager
	 * @return Set of queue family indices
	 */
	std::unordered_set<int> generateQueueFamilyIndexSet(const QueueManager &queueManager);
	
	/**
	 * @brief Creates and returns a new command resources instance containing
	 * a vector of newly allocated command pools for each different queue family
	 * index in a given set.
	 *
	 * @param[in,out] device Vulkan device
	 * @param[in] familyIndexSet Set of queue family indices
	 * @return New command resources
	 */
	CommandResources createCommandResources(const vk::Device &device,
											const std::unordered_set<int> &familyIndexSet);
	
	/**
	 * @brief Destroys a command resources instance and deallocates its command
	 * pools. The command resources will be invalid to use afterwards.
	 *
	 * @param[in,out] device Vulkan device
	 * @param[in,out] resources Command resources
	 */
	void destroyCommandResources(const vk::Device &device,
								 const CommandResources &resources);
	
	/**
	 * @brief Allocates and returns a new primary command buffer of a given
	 * command pool.
	 *
	 * @param[in,out] device Vulkan device
	 * @param[in,out] cmdPool Vulkan command pool
	 * @return New vulkan command buffer
	 */
	vk::CommandBuffer allocateCommandBuffer(const vk::Device &device,
											const vk::CommandPool &cmdPool);
	
	/**
	 * @brief Returns the matching command pool of given command resources for
	 * a specific queue.
	 *
	 * @param[in] queue Queue
	 * @param[in] cmdResources Command resources
	 * @return Command pool for a given queue
	 */
	vk::CommandPool chooseCmdPool(const Queue &queue,
								  const CommandResources &cmdResources);
	
	/**
	 * @brief Returns a queue of a given type from a queue manager.
	 *
	 * @param[in] type Type of queue
	 * @param[in] queueManager Queue manager
	 * @return Queue of a given type
	 */
	Queue getQueueForSubmit(QueueType type,
							const QueueManager &queueManager);
	
	/**
	 * @brief Begins the usage of a command buffer with given command buffer
	 * usage flags.
	 *
	 * @param[in] cmdBuffer Vulkan command buffer
	 * @param[in] flags Command buffer usage flags
	 */
	void beginCommandBuffer(const vk::CommandBuffer &cmdBuffer,
							vk::CommandBufferUsageFlags flags);

	/**
	 * @brief Submits a command buffer into a queue with given fence and
	 * semaphores to wait for or to signal after processing the command
	 * buffer.
	 *
	 * @param[in,out] queue Vulkan queue
	 * @param[in] cmdBuffer Vulkan command buffer
	 * @param[in] fence Vulkan fence to wait for
	 * @param[in] waitSemaphores Vector of semaphores to wait for
	 * @param[in] signalSemaphores Vector of semaphores to signal
	 */
	void submitCommandBufferToQueue(vk::Queue queue,
									const vk::CommandBuffer &cmdBuffer,
									const vk::Fence &fence,
									const std::vector<vk::Semaphore>& waitSemaphores,
									const std::vector<vk::Semaphore>& signalSemaphores);
	
}