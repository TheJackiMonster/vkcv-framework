/**
 * @authors Artur Wasmut
 * @file src/vkcv/Core.cpp
 * @brief Handling of global states regarding dependencies
 */

#include "vkcv/Core.hpp"
#include "PassManager.hpp"
#include "PipelineManager.hpp"

namespace vkcv
{

    /**
     * @brief The physical device is evaluated by three categories:
     * discrete GPU vs. integrated GPU, amount of queues and its abilities, and VRAM.physicalDevice.
     * @param physicalDevice The physical device
     * @return Device score as integer
    */
    int deviceScore(const vk::PhysicalDevice& physicalDevice)
    {
        int score = 0;
        vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        std::vector<vk::QueueFamilyProperties> qFamilyProperties = physicalDevice.getQueueFamilyProperties();

        // for every queue family compute queue flag bits and the amount of queues
        for (const auto& qFamily : qFamilyProperties) {
            uint32_t qCount = qFamily.queueCount;
            uint32_t bitCount = (static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eCompute) != 0)
                                + (static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eGraphics) != 0)
                                + (static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eTransfer) != 0)
                                + (static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eSparseBinding) != 0);
            score += static_cast<int>(qCount * bitCount);
        }

        // compute the VRAM of the physical device
        vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
        auto vram = static_cast<int>(memoryProperties.memoryHeaps[0].size / static_cast<uint32_t>(1E9));
        score *= vram;

        if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score *= 2;
        }
        else if (properties.deviceType != vk::PhysicalDeviceType::eIntegratedGpu) {
            score = -1;
        }

        return score;
    }

    /**
     * @brief All existing physical devices will be evaluated by deviceScore.
     * @param instance The instance
     * @return The optimal physical device
     * @see Context.deviceScore
    */
    vk::PhysicalDevice pickPhysicalDevice(vk::Instance& instance)
    {
        vk::PhysicalDevice phyDevice;
        std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

        if (devices.empty()) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        int max_score = -1;
        for (const auto& device : devices) {
            int score = deviceScore(device);
            if (score > max_score) {
                max_score = score;
                phyDevice = device;
            }
        }

        if (max_score == -1) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        return phyDevice;
    }


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
    void queueCreateInfosQueueHandles(vk::PhysicalDevice &physicalDevice,
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

        // DEBUG
        std::cout << "Input queue flags:" << std::endl;
        for (auto qFlag : queueFlags) {
            std::cout << "\t" << to_string(qFlag) << std::endl;
        }

        //check priorities of flags -> the lower prioCount the higher the priority
        std::vector<int> prios;
        for(auto flag: queueFlags){
            int prioCount = 0;
            for (int i = 0; i < qFamilyProperties.size(); i++) {
                prioCount += (static_cast<uint32_t>(flag & qFamilyProperties[i].queueFlags) != 0) * qFamilyProperties[i].queueCount;
            }
            prios.push_back(prioCount);
            std::cout<< "prio Count: " << prioCount << std::endl;
        }
        //resort flags with heighest priority before allocating the queues
        std::vector<vk::QueueFlagBits> newFlags;
        for(int i = 0; i < prios.size(); i++){
            auto minElem = std::min_element(prios.begin(), prios.end());
            int index = minElem - prios.begin();
            std::cout << "index: "<< index << std::endl;
            newFlags.push_back(queueFlags[index]);
            prios[index] = std::numeric_limits<int>::max();
        }

        std::cout << "Sorted queue flags:" << std::endl;
        for (auto qFlag : newFlags) {
            std::cout << "\t" << to_string(qFlag) << std::endl;
        }

        // create requested queues and check if more requested queues are supported
        // herefore: create vector that updates available queues in each queue family
        // structure: [qFamily_0, ..., qFamily_n] where
        // - qFamily_i = [GraphicsCount, ComputeCount, TransferCount], 0 <= i <= n
        std::vector<std::vector<int>> queueFamilyStatus, initialQueueFamilyStatus;

        for (auto qFamily : qFamilyProperties) {
            int graphicsCount = int(static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eGraphics) != 0) * qFamily.queueCount;
            int computeCount = int(static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eCompute) != 0) * qFamily.queueCount;;
            int transferCount = int(static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eTransfer) != 0) * qFamily.queueCount;;
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
                            std::cout << "Graphics queue available at queue family #" << i << std::endl;
                            found = true;
                        }
                    }
                    if (!found) {
                        throw std::runtime_error("Too many graphics queues were requested than being available!");
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
                            std::cout << "Compute queue available at queue family #" << i << std::endl;
                            found = true;
                        }
                    }
                    if (!found) {
                        throw std::runtime_error("Too many compute queues were requested than being available!");
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
                            std::cout << "Transfer queue available at queue family #" << i << std::endl;
                            found = true;
                        }
                    }
                    if (!found) {
                        throw std::runtime_error("Too many transfer queues were requested than being available!");
                    }
                    break;
                default:
                    throw std::runtime_error("Invalid input for queue flag bits. Valid inputs are 'vk::QueueFlagBits::eGraphics', 'vk::QueueFlagBits::eCompute' and 'vk::QueueFlagBits::eTransfer'.");
            }
        }

        std::cout << "Initial queue status:" << std::endl;
        int x = 0;
        for (std::vector<int> e : initialQueueFamilyStatus) {
            std::cout << "#" << x << ":\t[" << e[0] << ", " << e[1] << ", " << e[2] << "]" << std::endl;
            x++;
        }

        std::cout << "Actual queue status:" << std::endl;
        x = 0;
        for (std::vector<int> e : queueFamilyStatus) {
            std::cout << "#" << x << ":\t[" << e[0] << ", " << e[1] << ", " << e[2] << "]" << std::endl;
            x++;
        }

        // create all requested queues
        for (int i = 0; i < qFamilyProperties.size(); i++) {
            uint32_t create = std::abs(initialQueueFamilyStatus[i][0] - queueFamilyStatus[i][0]);
            std::cout << "For Queue Family #" << i << " create " << create << " queues" << std::endl;
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
     * @brief With the help of the reference "supported" all elements in "check" checked,
     * if they are supported by the physical device.
     * @param supported The reference that can be used to check "check"
     * @param check The elements to be checked
     * @return True, if all elements in "check" are supported
    */
    bool checkSupport(std::vector<const char*>& supported, std::vector<const char*>& check)
    {
        for (auto checkElem : check) {
            bool found = false;
            for (auto supportedElem : supported) {
                if (strcmp(supportedElem, checkElem) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }
        return true;
    }


    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        return extensions;
    }

    /**
     * Computes the queue handles from @p queuePairs
     * @param queuePairs The queuePairs that were created separately for each queue type (e.g., vk::QueueFlagBits::eGraphics)
     * @param device The device
     * @return An array of queue handles based on the @p queuePairs
     */
    std::vector<vk::Queue> getQueueHandles(const std::vector<std::pair<int, int>> queuePairs, const vk::Device device) {
        std::vector<vk::Queue> queueHandles;
        for (auto q : queuePairs) {
            int queueFamilyIndex = q.first; // the queueIndex of the queue family
            int queueIndex = q.second;   // the queueIndex within a queue family
            queueHandles.push_back(device.getQueue(queueFamilyIndex, queueIndex));
        }
        return queueHandles;
    }

    Core Core::create(const Window &window,
                      const char *applicationName,
                      uint32_t applicationVersion,
                      std::vector<vk::QueueFlagBits> queueFlags,
                      std::vector<const char *> instanceExtensions,
                      std::vector<const char *> deviceExtensions)
    {
        // check for layer support

        const std::vector<vk::LayerProperties>& layerProperties = vk::enumerateInstanceLayerProperties();

        std::vector<const char*> supportedLayers;
        supportedLayers.reserve(layerProperties.size());

        for (auto& elem : layerProperties) {
            supportedLayers.push_back(elem.layerName);
        }

// if in debug mode, check if validation layers are supported. Enable them if supported
#ifndef NDEBUG
        std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };

        if (!checkSupport(supportedLayers, validationLayers)) {
            throw std::runtime_error("Validation layers requested but not available!");
        }
#endif

        // check for extension support
        std::vector<vk::ExtensionProperties> instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();

        std::vector<const char*> supportedExtensions;
        supportedExtensions.reserve(instanceExtensionProperties.size());

        for (auto& elem : instanceExtensionProperties) {
            supportedExtensions.push_back(elem.extensionName);
        }

        if (!checkSupport(supportedExtensions, instanceExtensions)) {
            throw std::runtime_error("The requested instance extensions are not supported!");
        }

        // for GLFW: get all required extensions
        std::vector<const char*> requiredExtensions = getRequiredExtensions();
        instanceExtensions.insert(instanceExtensions.end(), requiredExtensions.begin(), requiredExtensions.end());

        const vk::ApplicationInfo applicationInfo(
                applicationName,
                applicationVersion,
                "vkCV",
                VK_MAKE_VERSION(0, 0, 1),
                VK_HEADER_VERSION_COMPLETE
        );

        vk::InstanceCreateInfo instanceCreateInfo(
                vk::InstanceCreateFlags(),
                &applicationInfo,
                0,
                nullptr,
                static_cast<uint32_t>(instanceExtensions.size()),
                instanceExtensions.data()
        );

#ifndef NDEBUG
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

        vk::Instance instance = vk::createInstance(instanceCreateInfo);

        std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
        vk::PhysicalDevice physicalDevice = pickPhysicalDevice(instance);

        // check for physical device extension support
        std::vector<vk::ExtensionProperties> deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
        supportedExtensions.clear();
        for (auto& elem : deviceExtensionProperties) {
            supportedExtensions.push_back(elem.extensionName);
        }
        if (!checkSupport(supportedExtensions, deviceExtensions)) {
            throw std::runtime_error("The requested device extensions are not supported by the physical device!");
        }

        //vector to define the queue priorities
        std::vector<float> qPriorities;
        qPriorities.resize(queueFlags.size(), 1.f); // all queues have the same priorities

        // create required queues
        std::vector<vk::DeviceQueueCreateInfo> qCreateInfos;
        std::vector<std::pair<int, int>> queuePairsGraphics, queuePairsCompute, queuePairsTransfer;
        queueCreateInfosQueueHandles(physicalDevice, qPriorities, queueFlags, qCreateInfos, queuePairsGraphics, queuePairsCompute, queuePairsTransfer);

        vk::DeviceCreateInfo deviceCreateInfo(
                vk::DeviceCreateFlags(),
                qCreateInfos.size(),
                qCreateInfos.data(),
                0,
                nullptr,
                deviceExtensions.size(),
                deviceExtensions.data(),
                nullptr		// Should our device use some features??? If yes: TODO
        );

#ifndef NDEBUG
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif


        vk::Device device = physicalDevice.createDevice(deviceCreateInfo);

        // maybe it can be useful to store these lists as member variable of Core
        std::vector<vk::Queue> graphicsQueues = getQueueHandles(queuePairsGraphics, device);
        std::vector<vk::Queue> computeQueues = getQueueHandles(queuePairsCompute, device);
        std::vector<vk::Queue> transferQueues = getQueueHandles(queuePairsTransfer, device);

        // examples for accessing queues
        vk::Queue graphicsQueue = graphicsQueues[0];
        vk::Queue computeQueue = computeQueues[0];
        vk::Queue transferQueue = transferQueues[0];

        Context context(instance, physicalDevice, device);

        SwapChain swapChain = SwapChain::create(window, context);

        std::vector<vk::Image> swapChainImages = device.getSwapchainImagesKHR(swapChain.getSwapchain());
        std::vector<vk::ImageView> imageViews;
        imageViews.reserve( swapChainImages.size() );
        //here can be swizzled with vk::ComponentSwizzle if needed
        vk::ComponentMapping componentMapping(
                vk::ComponentSwizzle::eR,
                vk::ComponentSwizzle::eG,
                vk::ComponentSwizzle::eB,
                vk::ComponentSwizzle::eA );

        vk::ImageSubresourceRange subResourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 );

        for ( auto image : swapChainImages )
        {
            vk::ImageViewCreateInfo imageViewCreateInfo(
                    vk::ImageViewCreateFlags(),
                    image,
                    vk::ImageViewType::e2D,
                    swapChain.getSurfaceFormat().format,
                    componentMapping,
                    subResourceRange
            );

            imageViews.push_back( device.createImageView( imageViewCreateInfo ) );
        }

        return Core(std::move(context) , window, swapChain, imageViews);
    }

    const Context &Core::getContext() const
    {
        return m_Context;
    }

    Core::Core(Context &&context, const Window &window , SwapChain swapChain,  std::vector<vk::ImageView> imageViews) noexcept :
            m_Context(std::move(context)),
            m_window(window),
            m_swapchain(swapChain),
            m_swapchainImageViews(imageViews),
			m_NextPipelineId(0),
			m_Pipelines{},
			m_PipelineLayouts{},
			m_PassManager{std::make_unique<PassManager>(m_Context.m_Device)},
			m_PipelineManager{std::make_unique<PipelineManager>(m_Context.m_Device)}
    {}

	Core::~Core() noexcept {
		std::cout << " Core " << std::endl;

		for(const auto &layout : m_PipelineLayouts)
        {
		    m_Context.m_Device.destroy(layout);
        }

		for(const auto &pipeline : m_Pipelines)
        {
		    m_Context.m_Device.destroy(pipeline);
        }

		m_PipelineLayouts.clear();
		m_Pipelines.clear();
		m_NextPipelineId = 0;

		for (auto image : m_swapchainImageViews) {
			m_Context.m_Device.destroyImageView(image);
		}

		m_Context.m_Device.destroySwapchainKHR(m_swapchain.getSwapchain());
		m_Context.m_Instance.destroySurfaceKHR(m_swapchain.getSurface());
	}

    PipelineHandle Core::createGraphicsPipeline(const PipelineConfig &config)
    {
        const vk::RenderPass &pass = m_PassManager->getVkPass(config.m_PassHandle);
        return m_PipelineManager->createPipeline(config, pass);
    }


    PassHandle Core::createPass(const PassConfig &config)
    {
        return m_PassManager->createPass(config);
    }

}
