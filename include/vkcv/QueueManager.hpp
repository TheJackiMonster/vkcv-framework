#pragma once
/**
 * @authors Sebastian Gaida, Tobias Frisch, Alexander Gauggel
 * @file vkcv/QueueManager.hpp
 * @brief Types to manage queues of a device.
 */
 
#include <vulkan/vulkan.hpp>

namespace vkcv {

	/**
	 * @brief Enum class to represent types of queues.
	 */
	enum class QueueType {
		Compute,
		Transfer,
		Graphics,
		Present
	};

	/**
	 * @brief Structure to represent a queue and its details.
	 */
	struct Queue {
		int familyIndex;
		int queueIndex;
		
		vk::Queue handle;
	};
	
	/**
	 * @brief Class to manage queues of a device.
	 */
	class QueueManager {
	public:
		/**
		 * @brief Creates a queue manager with the given pairs of queues.
		 *
		 * @param[in,out] device Vulkan device that holds the queues
		 * @param[in] queuePairsGraphics Graphic queue pairs of queueFamily and queueIndex
		 * @param[in] queuePairsCompute Compute queue pairs of queueFamily and queueIndex
		 * @param[in] queuePairsTransfer Transfer queue pairs of queueFamily and queueIndex
		 * @return New queue manager with the specified queue pairs
		 */
		static QueueManager create(vk::Device device,
								   const std::vector<std::pair<int, int>> &queuePairsGraphics,
								   const std::vector<std::pair<int, int>> &queuePairsCompute,
								   const std::vector<std::pair<int, int>> &queuePairsTransfer);
		
		/**
		 * @brief Returns the default queue with present support.
		 * Recommended to use the present queue in the swapchain.
		 *
		 * @return Default present queue
		 */
        [[nodiscard]]
        const Queue &getPresentQueue() const;

		/**
		 * @brief Returns all queues with the graphics flag.
		 *
		 * @return Vector of graphics queues
		 */
		[[nodiscard]]
		const std::vector<Queue> &getGraphicsQueues() const;

		/**
		 * @brief Returns all queues with the compute flag.
		 *
		 * @return Vector of compute queues
		 */
		[[nodiscard]]
        const std::vector<Queue> &getComputeQueues() const;

		/**
		 * @brief Returns all queues with the transfer flag.
		 *
		 * @return Vector of transfer queues
		 */
		[[nodiscard]]
        const std::vector<Queue> &getTransferQueues() const;
		
		/**
		 * @brief Checks for presenting support of a given surface
		 * in the queues and returns the queue family index of the
		 * supporting queue.
		 *
		 * @param[in] physicalDevice Vulkan physical device
		 * @param[in] surface Surface
		 * @return Queue family index of the supporting present queue
		 */
		static uint32_t checkSurfaceSupport(const vk::PhysicalDevice &physicalDevice,
											const vk::SurfaceKHR &surface);

    private:
        std::vector<Queue> m_graphicsQueues;
        std::vector<Queue> m_computeQueues;
        std::vector<Queue> m_transferQueues;
		
		size_t m_presentIndex;

        QueueManager(std::vector<Queue>&& graphicsQueues,
					 std::vector<Queue>&& computeQueues,
					 std::vector<Queue>&& transferQueues,
					 size_t presentIndex);
	};
}
