
#include <limits>
#include <unordered_set>
#include <iostream>

#include "vkcv/QueueManager.hpp"
#include "vkcv/Logger.hpp"
#include "vkcv/Swapchain.hpp"

namespace vkcv {

    /**
     * Computes the queue handles from @p queuePairs
     * @param device The device
     * @param queuePairs The queuePairs that were created separately for each queue type (e.g., vk::QueueFlagBits::eGraphics)
     * @return An array of queue handles based on the @p queuePairs
     */
    std::vector<Queue> getQueues(const vk::Device& device, const std::vector<std::pair<int, int>>& queuePairs) {
        std::vector<Queue> queues;
        
        for (auto q : queuePairs) {
            const int queueFamilyIndex = q.first; // the queueIndex of the queue family
            const int queueIndex = q.second;   // the queueIndex within a queue family
            
			queues.push_back({ queueFamilyIndex, queueIndex, device.getQueue(queueFamilyIndex, queueIndex) });
        }
        
        return queues;
    }


    QueueManager QueueManager::create(vk::Device device,
									  const std::vector<std::pair<int, int>> &queuePairsGraphics,
									  const std::vector<std::pair<int, int>> &queuePairsCompute,
									  const std::vector<std::pair<int, int>> &queuePairsTransfer) {

        std::vector<Queue> graphicsQueues = getQueues(device, queuePairsGraphics);
        std::vector<Queue> computeQueues  = getQueues(device, queuePairsCompute);
        std::vector<Queue> transferQueues = getQueues(device, queuePairsTransfer);

    	return QueueManager( std::move(graphicsQueues), std::move(computeQueues), std::move(transferQueues), 0);
	}

	uint32_t QueueManager::checkSurfaceSupport(const vk::PhysicalDevice &physicalDevice,
											   const vk::SurfaceKHR &surface) {
		std::vector<vk::QueueFamilyProperties> qFamilyProperties = physicalDevice.getQueueFamilyProperties();

		for(uint32_t i = 0; i < qFamilyProperties.size(); i++) {
			vk::Bool32 presentSupport;
			
			if ((vk::Result::eSuccess == physicalDevice.getSurfaceSupportKHR(i, surface, &presentSupport)) &&
				(presentSupport == VK_TRUE)) {
				return i;
			}
		}
		
		vkcv_log(LogLevel::WARNING, "No supported present queue");
		return 0;
	}

	QueueManager::QueueManager(std::vector<Queue>&& graphicsQueues, std::vector<Queue>&& computeQueues,
							   std::vector<Queue>&& transferQueues, size_t presentIndex)
	: m_graphicsQueues(graphicsQueues), m_computeQueues(computeQueues), m_transferQueues(transferQueues), m_presentIndex(presentIndex)
    {}

    const Queue &QueueManager::getPresentQueue() const {
        return m_graphicsQueues[m_presentIndex];
    }

    const std::vector<Queue> &QueueManager::getGraphicsQueues() const {
        return m_graphicsQueues;
    }

    const std::vector<Queue> &QueueManager::getComputeQueues() const {
        return m_computeQueues;
    }

    const std::vector<Queue> &QueueManager::getTransferQueues() const {
        return m_transferQueues;
    }

}