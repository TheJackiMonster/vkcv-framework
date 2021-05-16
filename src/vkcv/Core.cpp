/**
 * @authors Artur Wasmut
 * @file src/vkcv/Core.cpp
 * @brief Handling of global states regarding dependencies
 */

#include "vkcv/Core.hpp"
#include "PassManager.hpp"
#include "PipelineManager.hpp"
#include "Surface.hpp"
#include "ImageLayoutTransitions.hpp"
#include "Framebuffer.hpp"

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

		const vk::SurfaceKHR surface = createSurface(window.getWindow(), instance, physicalDevice);
		const QueueFamilyIndices queueFamilyIndices = getQueueFamilyIndices(physicalDevice, surface);
		std::vector<float> queuePriorities;
		const std::vector<vk::DeviceQueueCreateInfo> qCreateInfos = createDeviceQueueCreateInfo(queueFamilyIndices, &queuePriorities);

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
        Context context(instance, physicalDevice, device);

		const VulkanQueues queues = getDeviceQueues(device, queueFamilyIndices);

        SwapChain swapChain = SwapChain::create(window, context, surface);

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

		const int graphicQueueFamilyIndex = queueFamilyIndices.graphicsIndex;
		const auto defaultCommandResources = createDefaultCommandResources(context.getDevice(), graphicQueueFamilyIndex);
		const auto defaultSyncResources = createDefaultSyncResources(context.getDevice());

        return Core(std::move(context) , window, swapChain, imageViews, defaultCommandResources, defaultSyncResources, queues);
    }

    const Context &Core::getContext() const
    {
        return m_Context;
    }

	Core::Core(Context &&context, const Window &window , SwapChain swapChain,  std::vector<vk::ImageView> imageViews, 
		const CommandResources& commandResources, const SyncResources& syncResources, const VulkanQueues& queues) noexcept :
			m_Context(std::move(context)),
			m_window(window),
			m_swapchain(swapChain),
			m_swapchainImageViews(imageViews),
			m_PassManager{std::make_unique<PassManager>(m_Context.m_Device)},
			m_PipelineManager{std::make_unique<PipelineManager>(m_Context.m_Device)},
			m_CommandResources(commandResources),
			m_SyncResources(syncResources),
			m_Queues(queues)
	{}

	Core::~Core() noexcept {
		m_Context.getDevice().waitIdle();
		for (auto image : m_swapchainImageViews) {
			m_Context.m_Device.destroyImageView(image);
		}

		destroyCommandResources(m_Context.getDevice(), m_CommandResources);
		destroySyncResources(m_Context.getDevice(), m_SyncResources);
		destroyTemporaryFramebuffers();

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

	uint32_t Core::acquireSwapchainImage() {
		uint32_t index;
		m_Context.getDevice().acquireNextImageKHR(m_swapchain.getSwapchain(), 0, nullptr,
			m_SyncResources.swapchainImageAcquired, &index, {});
		const uint64_t timeoutPeriodNs = 1000;	// TODO: think if is adequate
		const auto& result = m_Context.getDevice().waitForFences(m_SyncResources.swapchainImageAcquired, true, timeoutPeriodNs);
		m_Context.getDevice().resetFences(m_SyncResources.swapchainImageAcquired);
		
		if (result == vk::Result::eTimeout) {
			index = std::numeric_limits<uint32_t>::max();
		}
		
		return index;
	}

	void Core::destroyTemporaryFramebuffers() {
		for (const vk::Framebuffer f : m_TemporaryFramebuffers) {
			m_Context.getDevice().destroyFramebuffer(f);
		}
		m_TemporaryFramebuffers.clear();
	}

	void Core::beginFrame() {
		m_currentSwapchainImageIndex = acquireSwapchainImage();
		
		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			std::cerr << "Drop frame!" << std::endl;
 			return;
		}
		
		m_Context.getDevice().waitIdle();	// FIMXE: this is a sin against graphics programming, but its getting late - Alex
		destroyTemporaryFramebuffers();
		const vk::CommandBufferUsageFlags beginFlags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		const vk::CommandBufferBeginInfo beginInfos(beginFlags);
		m_CommandResources.commandBuffer.begin(beginInfos);
	}

	void Core::renderTriangle(const PassHandle renderpassHandle, const PipelineHandle pipelineHandle, 
		const int width, const int height) {
		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}
  
		const vk::RenderPass renderpass = m_PassManager->getVkPass(renderpassHandle);
		const std::array<float, 4> clearColor = { 1.f, 1.f, 0.f, 1.f };
		const vk::ClearValue clearValues(clearColor);
		const vk::Rect2D renderArea(vk::Offset2D(0, 0), vk::Extent2D(width, height));
		const vk::ImageView imageView = m_swapchainImageViews[m_currentSwapchainImageIndex];
		const vk::Framebuffer framebuffer = createFramebuffer(m_Context.getDevice(), renderpass, width, height, imageView);
		m_TemporaryFramebuffers.push_back(framebuffer);
		const vk::RenderPassBeginInfo beginInfo(renderpass, framebuffer, renderArea, 1, &clearValues);
		const vk::SubpassContents subpassContents = {};
		m_CommandResources.commandBuffer.beginRenderPass(beginInfo, subpassContents, {});
		m_CommandResources.commandBuffer.endRenderPass();
	}

	void Core::endFrame() {
		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}
  
		const auto swapchainImages = m_Context.getDevice().getSwapchainImagesKHR(m_swapchain.getSwapchain());
		const vk::Image presentImage = swapchainImages[m_currentSwapchainImageIndex];

		m_CommandResources.commandBuffer.end();
		
		const vk::SubmitInfo submitInfo(0, nullptr, 0, 1, &(m_CommandResources.commandBuffer), 1, &m_SyncResources.renderFinished);
		m_Queues.graphicsQueue.submit(submitInfo);

		vk::Result presentResult;
		const vk::SwapchainKHR& swapchain = m_swapchain.getSwapchain();
		const vk::PresentInfoKHR presentInfo(1, &m_SyncResources.renderFinished, 1, &swapchain, 
			&m_currentSwapchainImageIndex, &presentResult);
		m_Queues.presentQueue.presentKHR(presentInfo);
		if (presentResult != vk::Result::eSuccess) {
			std::cout << "Error: swapchain present failed" << std::endl;
		}
	}

	vk::Format Core::getSwapchainImageFormat() {
		return m_swapchain.getImageFormat();
	}
}
