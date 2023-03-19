
#include <iostream>
#include <limits>
#include <unordered_set>

#include "vkcv/Logger.hpp"
#include "vkcv/QueueManager.hpp"

namespace vkcv {

	/**
	 * Computes the queue handles from @p queuePairs
	 * @param device The device
	 * @param queuePairs The queuePairs that were created separately for each queue type (e.g.,
	 * vk::QueueFlagBits::eGraphics)
	 * @return An array of queue handles based on the @p queuePairs
	 */
	Vector<Queue> getQueues(const vk::Device &device,
								 const Vector<std::pair<int, int>> &queuePairs) {
		Vector<Queue> queues;

		for (auto q : queuePairs) {
			const int queueFamilyIndex = q.first; // the queueIndex of the queue family
			const int queueIndex = q.second;      // the queueIndex within a queue family

			queues.push_back(
				{ queueFamilyIndex, queueIndex, device.getQueue(queueFamilyIndex, queueIndex) });
		}

		return queues;
	}

	QueueManager QueueManager::create(vk::Device device,
									  const Vector<std::pair<int, int>> &queuePairsGraphics,
									  const Vector<std::pair<int, int>> &queuePairsCompute,
									  const Vector<std::pair<int, int>> &queuePairsTransfer) {

		Vector<Queue> graphicsQueues = getQueues(device, queuePairsGraphics);
		Vector<Queue> computeQueues = getQueues(device, queuePairsCompute);
		Vector<Queue> transferQueues = getQueues(device, queuePairsTransfer);

		return QueueManager(std::move(graphicsQueues), std::move(computeQueues),
							std::move(transferQueues), 0);
	}

	uint32_t QueueManager::checkSurfaceSupport(const vk::PhysicalDevice &physicalDevice,
											   const vk::SurfaceKHR &surface) {
		Vector<vk::QueueFamilyProperties> qFamilyProperties =
			physicalDevice.getQueueFamilyProperties();

		for (uint32_t i = 0; i < qFamilyProperties.size(); i++) {
			vk::Bool32 presentSupport;

			if ((vk::Result::eSuccess
				 == physicalDevice.getSurfaceSupportKHR(i, surface, &presentSupport))
				&& (presentSupport == VK_TRUE)) {
				return i;
			}
		}

		vkcv_log(LogLevel::WARNING, "No supported present queue");
		return 0;
	}

	QueueManager::QueueManager(Vector<Queue> &&graphicsQueues,
							   Vector<Queue> &&computeQueues,
							   Vector<Queue> &&transferQueues, size_t presentIndex) :
		m_graphicsQueues(graphicsQueues),
		m_computeQueues(computeQueues), m_transferQueues(transferQueues),
		m_presentIndex(presentIndex) {}

	const Queue &QueueManager::getPresentQueue() const {
		return m_graphicsQueues [m_presentIndex];
	}

	const Vector<Queue> &QueueManager::getGraphicsQueues() const {
		return m_graphicsQueues;
	}

	const Vector<Queue> &QueueManager::getComputeQueues() const {
		return m_computeQueues;
	}

	const Vector<Queue> &QueueManager::getTransferQueues() const {
		return m_transferQueues;
	}

} // namespace vkcv