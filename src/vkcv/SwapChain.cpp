#include <vkcv/SwapChain.hpp>

namespace vkcv {

    SwapChain::SwapChain(vk::SurfaceKHR surface, vk::SwapchainKHR swapchain, vk::SurfaceFormatKHR format, uint32_t imageCount, vk::PresentModeKHR presentMode)
        : m_surface(surface), m_swapchain(swapchain), m_format( format), m_ImageCount(imageCount), m_presentMode(presentMode)
    {}

    const vk::SwapchainKHR& SwapChain::getSwapchain() const {
        return m_swapchain;
    }

    /**
     * gets surface of the swapchain
     * @return current surface
     */
    vk::SurfaceKHR SwapChain::getSurface() {
        return m_surface;
    }

    /**
     * gets the surface of the swapchain
     * @return chosen format
     */
    vk::SurfaceFormatKHR SwapChain::getSurfaceFormat(){
        return m_format;
    }

    /**
     * chooses Extent and clapms values to the available
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface of the swapchain
     * @param window of the current application
     * @return chosen Extent for the surface
     */
    vk::Extent2D chooseSwapExtent(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Window &window){
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        if(physicalDevice.getSurfaceCapabilitiesKHR(surface,&surfaceCapabilities) != vk::Result::eSuccess){
            throw std::runtime_error("cannot get surface capabilities. There is an issue with the surface.");
        }

        VkExtent2D extent2D = {
                static_cast<uint32_t>(window.getWidth()),
                static_cast<uint32_t>(window.getHeight())
        };
        extent2D.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent2D.width));
        extent2D.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent2D.height));

        if (extent2D.width > surfaceCapabilities.maxImageExtent.width ||
            extent2D.width < surfaceCapabilities.minImageExtent.width ||
            extent2D.height > surfaceCapabilities.maxImageExtent.height ||
            extent2D.height < surfaceCapabilities.minImageExtent.height) {
            std::printf("Surface size not matching. Resizing to allowed value.");
        }
        return extent2D;
    }

    /**
     * chooses Surface Format for the current surface
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface of the swapchain
     * @return available Format
     */
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
        uint32_t formatCount;
        physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, nullptr);
        std::vector<vk::SurfaceFormatKHR> availableFormats(formatCount);
        if (physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, &availableFormats[0]) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to get surface formats");
        }

        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm  && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    /**
     * returns vk::PresentModeKHR::eMailbox if available or vk::PresentModeKHR::eFifo otherwise
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface of the swapchain
     * @return available PresentationMode
     */
    vk::PresentModeKHR choosePresentMode(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
        uint32_t modeCount;
        physicalDevice.getSurfacePresentModesKHR( surface, &modeCount, nullptr );
        std::vector<vk::PresentModeKHR> availablePresentModes(modeCount);
        if (physicalDevice.getSurfacePresentModesKHR(surface, &modeCount, &availablePresentModes[0]) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to get presentation modes");
        }

        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        // The FIFO present mode is guaranteed by the spec to be supported
        return vk::PresentModeKHR::eFifo;
    }

    /**
     * returns the minImageCount +1 for at least doublebuffering, if it's greater than maxImageCount return maxImageCount
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface of the swapchain
     * @return available ImageCount
     */
    uint32_t chooseImageCount(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        if(physicalDevice.getSurfaceCapabilitiesKHR(surface, &surfaceCapabilities) != vk::Result::eSuccess){
            throw std::runtime_error("cannot get surface capabilities. There is an issue with the surface.");
        }

        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;    // minImageCount should always be at least 2; set to 3 for triple buffering
        // check if requested image count is supported
        if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
            imageCount = surfaceCapabilities.maxImageCount;
        }

        return imageCount;
    }
    /**
     * creates and returns a swapchain with default specs
     * @param window of the current application
     * @param context that keeps instance, physicalDevice and a device.
     * @return swapchain
     */
    SwapChain SwapChain::create(const Window &window, const Context &context, const vk::SurfaceKHR surface) {
        const vk::Instance& instance = context.getInstance();
        const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
        const vk::Device& device = context.getDevice();

        vk::Extent2D extent2D = chooseSwapExtent(physicalDevice, surface, window);
        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(physicalDevice, surface);
        vk::PresentModeKHR presentMode = choosePresentMode(physicalDevice, surface);
        uint32_t imageCount = chooseImageCount(physicalDevice, surface);

        vk::SwapchainCreateInfoKHR swapchainCreateInfo(
                vk::SwapchainCreateFlagsKHR(),  //flags
                surface,    // surface
                imageCount,  // minImageCount TODO: how many do we need for our application?? "must be less than or equal to the value returned in maxImageCount" -> 3 for Triple Buffering, else 2 for Double Buffering (should be the standard)
                surfaceFormat.format,   // imageFormat
                surfaceFormat.colorSpace,   // imageColorSpace
                extent2D,   // imageExtent
                1,  // imageArrayLayers TODO: should we only allow non-stereoscopic applications? yes -> 1, no -> ? "must be greater than 0, less or equal to maxImageArrayLayers"
                vk::ImageUsageFlagBits::eColorAttachment,  // imageUsage TODO: what attachments? only color? depth?
                vk::SharingMode::eExclusive,    // imageSharingMode TODO: which sharing mode? "VK_SHARING_MODE_EXCLUSIV access exclusive to a single queue family, better performance", "VK_SHARING_MODE_CONCURRENT access from multiple queues"
                0,  // queueFamilyIndexCount, the number of queue families having access to the image(s) of the swapchain when imageSharingMode is VK_SHARING_MODE_CONCURRENT
                nullptr,    // pQueueFamilyIndices, the pointer to an array of queue family indices having access to the images(s) of the swapchain when imageSharingMode is VK_SHARING_MODE_CONCURRENT
                vk::SurfaceTransformFlagBitsKHR::eIdentity, // preTransform, transformations applied onto the image before display
                vk::CompositeAlphaFlagBitsKHR::eOpaque, // compositeAlpha, TODO: how to handle transparent pixels? do we need transparency? If no -> opaque
                presentMode,    // presentMode
                true,   // clipped
                nullptr // oldSwapchain
        );

        vk::SwapchainKHR swapchain = device.createSwapchainKHR(swapchainCreateInfo);

        return SwapChain(surface, swapchain, surfaceFormat, imageCount, presentMode);
    }

    vk::SwapchainKHR SwapChain::recreateSwapchain( const Context &context, const Window &window, int width, int height){
        vk::SwapchainKHR oldSwapchain = m_swapchain;
        vk::Extent2D extent2D = chooseSwapExtent(context.getPhysicalDevice(), m_surface, window);

        vk::SwapchainCreateInfoKHR swapchainCreateInfo(
                vk::SwapchainCreateFlagsKHR(),
                m_surface,
                m_ImageCount,
                m_format.format,
                m_format.colorSpace,
                extent2D,
                1,
                vk::ImageUsageFlagBits::eColorAttachment,
                vk::SharingMode::eExclusive,
                0,
                nullptr,
                vk::SurfaceTransformFlagBitsKHR::eIdentity,
                vk::CompositeAlphaFlagBitsKHR::eOpaque,
                m_presentMode,
                true,
                oldSwapchain
        );
        m_swapchain = context.getDevice().createSwapchainKHR(swapchainCreateInfo);
    }


    SwapChain::~SwapChain() {
        // needs to be destroyed by creator
    }

	uint32_t SwapChain::getImageCount() {
		return m_ImageCount;
	}
}
