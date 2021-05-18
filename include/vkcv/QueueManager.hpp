#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv {
	class QueueManager {
	public:
		static QueueManager create(vk::Device device,
                            std::vector<std::pair<int, int>> &queuePairsGraphics,
                            std::vector<std::pair<int, int>> &queuePairsCompute,
                            std::vector<std::pair<int, int>> &queuePairsTransfer);

        const vk::Queue &getPresentQueue() const;

		const std::vector<vk::Queue> &getGraphicsQueues() const;

        const std::vector<vk::Queue> &getComputeQueues() const;

        const std::vector<vk::Queue> &getTransferQueues() const;

        static void queueCreateInfosQueueHandles(vk::PhysicalDevice &physicalDevice,
                std::vector<float> &queuePriorities,
                std::vector<vk::QueueFlagBits> &queueFlags,
                std::vector<vk::DeviceQueueCreateInfo> &queueCreateInfos,
                std::vector<std::pair<int, int>> &queuePairsGraphics,
                std::vector<std::pair<int, int>> &queuePairsCompute,
                std::vector<std::pair<int, int>> &queuePairsTransfer);

    private:
        vk::Queue m_presentQueue;
        std::vector<vk::Queue> m_graphicsQueues;
        std::vector<vk::Queue> m_computeQueues;
        std::vector<vk::Queue> m_transferQueues;

        QueueManager(std::vector<vk::Queue> graphicsQueues, std::vector<vk::Queue> computeQueues, std::vector<vk::Queue> transferQueues, vk::Queue presentQueue);
	};
}
