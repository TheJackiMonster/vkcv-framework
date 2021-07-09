#include <vkcv/Swapchain.hpp>
#include <utility>

#include <GLFW/glfw3.h>

namespace vkcv
{
    /**
    * creates surface and checks availability
    * @param window current window for the surface
    * @param instance Vulkan-Instance
    * @param physicalDevice Vulkan-PhysicalDevice
    * @return created surface
    */
    vk::SurfaceKHR createSurface(GLFWwindow* window, const vk::Instance& instance, const vk::PhysicalDevice& physicalDevice) {
        //create surface
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create a window surface!");
        }
        vk::Bool32 surfaceSupport = false;
        if (physicalDevice.getSurfaceSupportKHR(0, vk::SurfaceKHR(surface), &surfaceSupport) != vk::Result::eSuccess && surfaceSupport != true) {
            throw std::runtime_error("surface is not supported by the device!");
        }

        return vk::SurfaceKHR(surface);
    }

    Swapchain::Swapchain(const Surface &surface,
                         vk::SwapchainKHR swapchain,
                         vk::Format format,
                         vk::ColorSpaceKHR colorSpace,
                         vk::PresentModeKHR presentMode,
                         uint32_t imageCount,
						 vk::Extent2D extent) noexcept :
			m_Surface(surface),
			m_Swapchain(swapchain),
			m_Format(format),
			m_ColorSpace(colorSpace),
			m_PresentMode(presentMode),
			m_ImageCount(imageCount),
			m_Extent(extent),
			m_RecreationRequired(false)
    {}
    
    Swapchain::Swapchain(const Swapchain &other) :
			m_Surface(other.m_Surface),
			m_Swapchain(other.m_Swapchain),
			m_Format(other.m_Format),
			m_ColorSpace(other.m_ColorSpace),
			m_PresentMode(other.m_PresentMode),
			m_ImageCount(other.m_ImageCount),
			m_Extent(other.m_Extent),
			m_RecreationRequired(other.m_RecreationRequired.load())
	{}

    const vk::SwapchainKHR& Swapchain::getSwapchain() const {
        return m_Swapchain;
    }

    vk::SurfaceKHR Swapchain::getSurface() const {
        return m_Surface.handle;
    }

    vk::Format Swapchain::getFormat() const{
        return m_Format;
    }

    /**
     * chooses Extent and clapms values to the available
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface of the swapchain
     * @param window of the current application
     * @return chosen Extent for the surface
     */
    vk::Extent2D chooseExtent(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Window &window){
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        if(physicalDevice.getSurfaceCapabilitiesKHR(surface,&surfaceCapabilities) != vk::Result::eSuccess){
            throw std::runtime_error("cannot get surface capabilities. There is an issue with the surface.");
        }
        
        int fb_width, fb_height;
        window.getFramebufferSize(fb_width, fb_height);
        
        VkExtent2D extent2D = {
                static_cast<uint32_t>(fb_width),
                static_cast<uint32_t>(fb_height)
        };
        
        extent2D.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent2D.width));
        extent2D.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent2D.height));

        return extent2D;
    }

    /**
     * chooses Surface Format for the current surface
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface of the swapchain
     * @return available Format
     */
    vk::SurfaceFormatKHR chooseSurfaceFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
        std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);

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
        std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

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
        vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	
		// minImageCount should always be at least 2; set to 3 for triple buffering
        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        
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
    Swapchain Swapchain::create(const Window &window, const Context &context) {
        const vk::Instance& instance = context.getInstance();
        const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
        const vk::Device& device = context.getDevice();

        Surface surface;
        surface.handle       = createSurface(window.getWindow(), instance, physicalDevice);
        surface.formats      = physicalDevice.getSurfaceFormatsKHR(surface.handle);
        surface.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface.handle);
        surface.presentModes = physicalDevice.getSurfacePresentModesKHR(surface.handle);

        vk::Extent2D chosenExtent = chooseExtent(physicalDevice, surface.handle, window);
        vk::SurfaceFormatKHR chosenSurfaceFormat = chooseSurfaceFormat(physicalDevice, surface.handle);
        vk::PresentModeKHR chosenPresentMode = choosePresentMode(physicalDevice, surface.handle);
        uint32_t chosenImageCount = chooseImageCount(physicalDevice, surface.handle);

        vk::SwapchainCreateInfoKHR swapchainCreateInfo(
                vk::SwapchainCreateFlagsKHR(),  //flags
                surface.handle,    // surface
                chosenImageCount,  // minImageCount TODO: how many do we need for our application?? "must be less than or equal to the value returned in maxImageCount" -> 3 for Triple Buffering, else 2 for Double Buffering (should be the standard)
                chosenSurfaceFormat.format,   // imageFormat
                chosenSurfaceFormat.colorSpace,   // imageColorSpace
                chosenExtent,   // imageExtent
                1,  // imageArrayLayers TODO: should we only allow non-stereoscopic applications? yes -> 1, no -> ? "must be greater than 0, less or equal to maxImageArrayLayers"
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage,  // imageUsage TODO: what attachments? only color? depth?
                vk::SharingMode::eExclusive,    // imageSharingMode TODO: which sharing mode? "VK_SHARING_MODE_EXCLUSIV access exclusive to a single queue family, better performance", "VK_SHARING_MODE_CONCURRENT access from multiple queues"
                0,  // queueFamilyIndexCount, the number of queue families having access to the image(s) of the swapchain when imageSharingMode is VK_SHARING_MODE_CONCURRENT
                nullptr,    // pQueueFamilyIndices, the pointer to an array of queue family indices having access to the images(s) of the swapchain when imageSharingMode is VK_SHARING_MODE_CONCURRENT
                vk::SurfaceTransformFlagBitsKHR::eIdentity, // preTransform, transformations applied onto the image before display
                vk::CompositeAlphaFlagBitsKHR::eOpaque, // compositeAlpha, TODO: how to handle transparent pixels? do we need transparency? If no -> opaque
                chosenPresentMode,    // presentMode
                true,   // clipped
                nullptr // oldSwapchain
        );

        vk::SwapchainKHR swapchain = device.createSwapchainKHR(swapchainCreateInfo);

        return Swapchain(surface,
                         swapchain,
                         chosenSurfaceFormat.format,
                         chosenSurfaceFormat.colorSpace,
                         chosenPresentMode,
                         chosenImageCount,
						 chosenExtent);
    }
    
    bool Swapchain::shouldUpdateSwapchain() const {
    	return m_RecreationRequired;
    }
    
    void Swapchain::updateSwapchain(const Context &context, const Window &window) {
    	if (!m_RecreationRequired.exchange(false)) {
			return;
		}
    	
		vk::SwapchainKHR oldSwapchain = m_Swapchain;
		vk::Extent2D extent2D = chooseExtent(context.getPhysicalDevice(), m_Surface.handle, window);
	
		if ((extent2D.width >= MIN_SWAPCHAIN_SIZE) && (extent2D.height >= MIN_SWAPCHAIN_SIZE)) {
			vk::SwapchainCreateInfoKHR swapchainCreateInfo(
					vk::SwapchainCreateFlagsKHR(),
					m_Surface.handle,
					m_ImageCount,
					m_Format,
					m_ColorSpace,
					extent2D,
					1,
					vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage,
					vk::SharingMode::eExclusive,
					0,
					nullptr,
					vk::SurfaceTransformFlagBitsKHR::eIdentity,
					vk::CompositeAlphaFlagBitsKHR::eOpaque,
					m_PresentMode,
					true,
					oldSwapchain
			);
			
			m_Swapchain = context.getDevice().createSwapchainKHR(swapchainCreateInfo);
		} else {
			m_Swapchain = nullptr;
			
			signalSwapchainRecreation();
		}
		
		if (oldSwapchain) {
			context.getDevice().destroySwapchainKHR(oldSwapchain);
		}
		
		m_Extent = extent2D;
    }

    void Swapchain::signalSwapchainRecreation() {
		m_RecreationRequired = true;
    }
    
    const vk::Extent2D& Swapchain::getExtent() const {
    	return m_Extent;
    }

    Swapchain::~Swapchain() {
        // needs to be destroyed by creator
    }

	uint32_t Swapchain::getImageCount() const {
		return m_ImageCount;
	}
}
