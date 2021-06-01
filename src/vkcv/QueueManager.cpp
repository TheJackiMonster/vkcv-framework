
#include <limits>
#include <unordered_set>

#include "vkcv/QueueManager.hpp"


namespace vkcv {

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
    void QueueManager::queueCreateInfosQueueHandles(vk::PhysicalDevice &physicalDevice,
                                      std::vector<float> &queuePriorities,
                                      std::vector<vk::QueueFlagBits> &queueFlags,
                                      std::vector<vk::DeviceQueueCreateInfo> &queueCreateInfos,
                                      std::vector<std::pair<int, int>> &queuePairsGraphics,
                                      std::vector<std::pair<int, int>> &queuePairsCompute,
                                      std::vector<std::pair<int, int>> &queuePairsTransfer)
    {
        queueCreateInfos = {};
        queuePairsGraphics = {};
        queuePairsCompute = {};
        queuePairsTransfer = {};
        std::vector<vk::QueueFamilyProperties> qFamilyProperties = physicalDevice.getQueueFamilyProperties();

        //check priorities of flags -> the lower prioCount the higher the priority
        std::vector<int> prios;
        for(auto flag: queueFlags) {
            int prioCount = 0;
            for (int i = 0; i < qFamilyProperties.size(); i++) {
                prioCount += (static_cast<uint32_t>(flag & qFamilyProperties[i].queueFlags) != 0) * qFamilyProperties[i].queueCount;
            }
            prios.push_back(prioCount);
        }
        //resort flags with heighest priority before allocating the queues
        std::vector<vk::QueueFlagBits> newFlags;
        for(int i = 0; i < prios.size(); i++) {
            auto minElem = std::min_element(prios.begin(), prios.end());
            int index = minElem - prios.begin();
            newFlags.push_back(queueFlags[index]);
            prios[index] = std::numeric_limits<int>::max();
        }

        // create requested queues and check if more requested queues are supported
        // herefore: create vector that updates available queues in each queue family
        // structure: [qFamily_0, ..., qFamily_n] where
        // - qFamily_i = [GraphicsCount, ComputeCount, TransferCount], 0 <= i <= n
        std::vector<std::vector<int>> queueFamilyStatus, initialQueueFamilyStatus;

        for (auto qFamily : qFamilyProperties) {
            int graphicsCount = int(static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eGraphics) != 0) * qFamily.queueCount;
            int computeCount = int(static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eCompute) != 0) * qFamily.queueCount;
            int transferCount = int(static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eTransfer) != 0) * qFamily.queueCount;
            queueFamilyStatus.push_back({graphicsCount, computeCount, transferCount});
        }

        initialQueueFamilyStatus = queueFamilyStatus;
        // check if every queue with the specified queue flag can be created
        // this automatically checks for queue flag support!
        for (auto qFlag : newFlags) {
            bool found;
            switch (qFlag) {
                case vk::QueueFlagBits::eGraphics:
                    found = false;
                    for (int i = 0; i < queueFamilyStatus.size() && !found; i++) {
                        if (queueFamilyStatus[i][0] > 0) {
                            queuePairsGraphics.push_back(std::pair(i, initialQueueFamilyStatus[i][0] - queueFamilyStatus[i][0]));
                            queueFamilyStatus[i][0]--;
                            queueFamilyStatus[i][1]--;
                            queueFamilyStatus[i][2]--;
                            found = true;
                        }
                    }
                    if (!found) {
                        for (int i = 0; i < queueFamilyStatus.size() && !found; i++) {
                            if (initialQueueFamilyStatus[i][0] > 0) {
                                queuePairsGraphics.push_back(std::pair(i, 0));
                                found = true;
                            }
                        }
                    }
                    break;
                case vk::QueueFlagBits::eCompute:
                    found = false;
                    for (int i = 0; i < queueFamilyStatus.size() && !found; i++) {
                        if (queueFamilyStatus[i][1] > 0) {
                            queuePairsCompute.push_back(std::pair(i, initialQueueFamilyStatus[i][1] - queueFamilyStatus[i][1]));
                            queueFamilyStatus[i][0]--;
                            queueFamilyStatus[i][1]--;
                            queueFamilyStatus[i][2]--;
                            found = true;
                        }
                    }
                    if (!found) {
                        for (int i = 0; i < queueFamilyStatus.size() && !found; i++) {
                            if (initialQueueFamilyStatus[i][1] > 0) {
                                queuePairsCompute.push_back(std::pair(i, 0));
                                found = true;
                            }
                        }
                    }
                    break;
                case vk::QueueFlagBits::eTransfer:
                    found = false;
                    for (int i = 0; i < queueFamilyStatus.size() && !found; i++) {
                        if (queueFamilyStatus[i][2] > 0) {
                            queuePairsTransfer.push_back(std::pair(i, initialQueueFamilyStatus[i][2] - queueFamilyStatus[i][2]));
                            queueFamilyStatus[i][0]--;
                            queueFamilyStatus[i][1]--;
                            queueFamilyStatus[i][2]--;
                            found = true;
                        }
                    }
                    if (!found) {
                        for (int i = 0; i < queueFamilyStatus.size() && !found; i++) {
                            if (initialQueueFamilyStatus[i][2] > 0) {
                                queuePairsTransfer.push_back(std::pair(i, 0));
                                found = true;
                            }
                        }
                    }
                    break;
                default:
                    throw std::runtime_error("Invalid input for queue flag bits. Valid inputs are 'vk::QueueFlagBits::eGraphics', 'vk::QueueFlagBits::eCompute' and 'vk::QueueFlagBits::eTransfer'.");
            }
        }

        // create all requested queues
        for (int i = 0; i < qFamilyProperties.size(); i++) {
            uint32_t create = std::abs(initialQueueFamilyStatus[i][0] - queueFamilyStatus[i][0]);
            if (create > 0) {
                vk::DeviceQueueCreateInfo qCreateInfo(
                        vk::DeviceQueueCreateFlags(),
                        i,
                        create,
                        queuePriorities.data()
                );
                queueCreateInfos.push_back(qCreateInfo);
            }
        }
    }

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
                                      std::vector<std::pair<int, int>> &queuePairsGraphics,
                                      std::vector<std::pair<int, int>> &queuePairsCompute,
                                      std::vector<std::pair<int, int>> &queuePairsTransfer) {

        std::vector<Queue> graphicsQueues = getQueues(device, queuePairsGraphics);
        std::vector<Queue> computeQueues = getQueues(device, queuePairsCompute );
        std::vector<Queue> transferQueues = getQueues(device, queuePairsTransfer);

    	return QueueManager( std::move(graphicsQueues), std::move(computeQueues), std::move(transferQueues), 0);
	}

	QueueManager::QueueManager(std::vector<Queue>&& graphicsQueues, std::vector<Queue>&& computeQueues, std::vector<Queue>&& transferQueues, size_t presentIndex)
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