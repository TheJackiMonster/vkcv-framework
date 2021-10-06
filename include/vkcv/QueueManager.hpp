#pragma once
/**
 * @authors Sebastian Gaida, Tobias Frisch, Alexander Gauggel
 * @file vkcv/QueueManager.hpp
 * @brief Types to manage queues of a device.
 */
 
#include <vulkan/vulkan.hpp>

namespace vkcv {

	/**
	 * Enum class to represent types of queues.
	 */
	enum class QueueType {
		Compute,
		Transfer,
		Graphics,
		Present
	};

	/**
	 * Struct to represent a queue, provide its family and its index.
	 */
	struct Queue {
		int familyIndex;
		int queueIndex;
		
		vk::Queue handle;
	};
	
	/**
	 * Class to manage queues of a device.
	 */
	class QueueManager {
	public:
		/**
		 * Creates a QueueManager with the given queue pairs
		 * @param device device that holds the queues that are specified in the queue pairs
		 * @param queuePairsGraphics graphic queue pairs of queueFamily and queueIndex
		 * @param queuePairsCompute compute queue pairs of queueFamily and queueIndex
		 * @param queuePairsTransfer transfer queue pairs of queueFamily and queueIndex
		 * @return a QueueManager with the specified queuePairs
		 */
		static QueueManager create(vk::Device device,
                            std::vector<std::pair<int, int>> &queuePairsGraphics,
                            std::vector<std::pair<int, int>> &queuePairsCompute,
                            std::vector<std::pair<int, int>> &queuePairsTransfer);
		/**
		 * Returns the default presentQueue. Recommended to use the presentQueue in the Swapchain
		 * @return a default presentQueue
		 */
        [[nodiscard]]
        const Queue &getPresentQueue() const;

		/**
		 * Returns all queues with the graphics flag
		 * @return vector of graphic queues
		 */
		[[nodiscard]]
		const std::vector<Queue> &getGraphicsQueues() const;

		/**
		 * Returns all queues with the compute flag
		 * @return vector of compute queues
		 */
		[[nodiscard]]
        const std::vector<Queue> &getComputeQueues() const;

		/**
		 * Returns all queues with the transfer flag
		 * @return vector of transfer queues
		 */
		[[nodiscard]]
        const std::vector<Queue> &getTransferQueues() const;

		/**
		 * Given the @p physicalDevice and the @p queuePriorities, the @p queueCreateInfos are computed. First, the requested
		 * queues are sorted by priority depending on the availability of queues in the queue families of the given
		 * @p physicalDevice. Then check, if all requested queues are creatable. If so, the @p queueCreateInfos will be computed.
		 * Furthermore, lists of index pairs (queueFamilyIndex, queueIndex) for later referencing of the separate queues will
		 * be computed.
		 * @param[in] physicalDevice The physical device
		 * @param[in] queuePriorities The queue priorities used for the computation of @p queueCreateInfos
		 * @param[in] queueFlags The queue flags requesting the queues
		 * @param[in,out] queueCreateInfos The queue create info structures to be created
		 * @param[in,out] queuePairsGraphics The list of index pairs (queueFamilyIndex, queueIndex) of queues of type
		 *      vk::QueueFlagBits::eGraphics
		 * @param[in,out] queuePairsCompute The list of index pairs (queueFamilyIndex, queueIndex) of queues of type
		 *      vk::QueueFlagBits::eCompute
		 * @param[in,out] queuePairsTransfer The list of index pairs (queueFamilyIndex, queueIndex) of queues of type
		 *      vk::QueueFlagBits::eTransfer
		 * @throws std::runtime_error If the requested queues from @p queueFlags are not creatable due to insufficient availability.
		 */
        static void queueCreateInfosQueueHandles(vk::PhysicalDevice &physicalDevice,
                const std::vector<float> &queuePriorities,
                const std::vector<vk::QueueFlagBits> &queueFlags,
                std::vector<vk::DeviceQueueCreateInfo> &queueCreateInfos,
                std::vector<std::pair<int, int>> &queuePairsGraphics,
                std::vector<std::pair<int, int>> &queuePairsCompute,
                std::vector<std::pair<int, int>> &queuePairsTransfer);

		/**
		 * Checks for surface support in the queues
		 * @param physicalDevice to get the Queues
		 * @param surface that needs to checked
		 * @return
		 */
		static uint32_t checkSurfaceSupport(const vk::PhysicalDevice &physicalDevice, vk::SurfaceKHR &surface);

    private:
        std::vector<Queue> m_graphicsQueues;
        std::vector<Queue> m_computeQueues;
        std::vector<Queue> m_transferQueues;
		
		size_t m_presentIndex;

        QueueManager(std::vector<Queue>&& graphicsQueues, std::vector<Queue>&& computeQueues, std::vector<Queue>&& transferQueues, size_t presentIndex);
	};
}
