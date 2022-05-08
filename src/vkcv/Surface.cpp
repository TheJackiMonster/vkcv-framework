
#include <vkcv/Surface.hpp>
#include <vkcv/Logger.hpp>

#include <GLFW/glfw3.h>

namespace vkcv {
	
	/**
    * @brief Creates vulkan surface and checks availability.
    *
    * @param[in,out] window Current window for the surface
    * @param[in,out] instance Vulkan-Instance
    * @param[in,out] physicalDevice Vulkan-PhysicalDevice
    * @param[out] surface Vulkan-Surface
    * @return Created vulkan surface
    */
	bool createVulkanSurface(GLFWwindow* window,
							 const vk::Instance& instance,
							 const vk::PhysicalDevice& physicalDevice,
							 vk::SurfaceKHR& surface) {
		VkSurfaceKHR api_surface;
		
		if (glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &api_surface) != VK_SUCCESS) {
			vkcv_log(LogLevel::ERROR, "Failed to create a window surface");
			return false;
		}
		
		vk::Bool32 surfaceSupport = false;
		surface = vk::SurfaceKHR(api_surface);
		
		if ((physicalDevice.getSurfaceSupportKHR(0, surface, &surfaceSupport) != vk::Result::eSuccess) ||
			(!surfaceSupport)) {
			vkcv_log(LogLevel::ERROR, "Surface is not supported by the device");
			instance.destroy(surface);
			surface = nullptr;
			return false;
		}
		
		return true;
	}
	
	/**
     * @brief Chooses an Extent and clamps values to the available capabilities.
     *
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface Vulkan-Surface of the swapchain
     * @param window Window of the current application
     * @return Chosen Extent for the surface
     */
	vk::Extent2D chooseExtent(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, const Window &window) {
		int fb_width, fb_height;
		window.getFramebufferSize(fb_width, fb_height);
		
		VkExtent2D extent2D = {
				static_cast<uint32_t>(fb_width),
				static_cast<uint32_t>(fb_height)
		};
		
		vk::SurfaceCapabilitiesKHR surfaceCapabilities;
		if(physicalDevice.getSurfaceCapabilitiesKHR(surface, &surfaceCapabilities) != vk::Result::eSuccess) {
			vkcv_log(LogLevel::WARNING, "The capabilities of the surface can not be retrieved");
			
			extent2D.width = std::max(MIN_SURFACE_SIZE, extent2D.width);
			extent2D.height = std::max(MIN_SURFACE_SIZE, extent2D.height);
		} else {
			extent2D.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent2D.width));
			extent2D.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent2D.height));
		}
		
		return extent2D;
	}
	
	/**
     * @brief Chooses Surface Format for the current surface
     *
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface Vulkan-Surface of the swapchain
     * @return Available Format
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
     * @brief Returns vk::PresentModeKHR::eMailbox if available or
     * vk::PresentModeKHR::eFifo otherwise
     *
     * @param physicalDevice Vulkan-PhysicalDevice
     * @param surface Vulkan-Surface of the swapchain
     * @return Available PresentationMode
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
	* @brief Returns the minImageCount +1 for at least double buffering,
	* if it's greater than maxImageCount return maxImageCount
	*
	* @param physicalDevice Vulkan-PhysicalDevice
	* @param surface Vulkan-Surface of the swapchain
	* @return Available image count
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
	
	Surface::Surface(const Context &context,
					 const vk::SurfaceKHR &handle,
					 uint32_t presentQueueIndex,
					 const vk::Extent2D &extent,
					 vk::Format format,
					 vk::ColorSpaceKHR colorSpace)
					 : m_Context(&context),
					   m_Handle(handle),
					   m_PresentQueueIndex(presentQueueIndex),
					   m_Extent(extent),
					   m_Format(format),
					   m_ColorSpace(colorSpace) {
	}
	
	vk::SwapchainKHR Surface::createVulkanSwapchain(const Window &window,
													const vk::SwapchainKHR &oldSwapchain) {
		if ((!m_Context) || (!m_Handle))
			return nullptr;
		
		const vk::PhysicalDevice& physicalDevice = m_Context->getPhysicalDevice();
		const vk::Device& device = m_Context->getDevice();
		
		m_Extent = chooseExtent(physicalDevice, m_Handle, window);
		
		if ((m_Extent.width < MIN_SURFACE_SIZE) || (m_Extent.height < MIN_SURFACE_SIZE)) {
			return nullptr;
		}
		
		vk::SurfaceFormatKHR chosenSurfaceFormat = chooseSurfaceFormat(physicalDevice, m_Handle);
		vk::PresentModeKHR chosenPresentMode = choosePresentMode(physicalDevice, m_Handle);
		uint32_t chosenImageCount = chooseImageCount(physicalDevice, m_Handle);
		
		m_Format = chosenSurfaceFormat.format;
		m_ColorSpace = chosenSurfaceFormat.colorSpace;
		
		vk::SwapchainCreateInfoKHR swapchainCreateInfo (
				vk::SwapchainCreateFlagsKHR(),
				m_Handle,
				chosenImageCount,
				m_Format,
				m_ColorSpace,
				m_Extent,
				1,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage,
				vk::SharingMode::eExclusive,
				0,
				nullptr,
				vk::SurfaceTransformFlagBitsKHR::eIdentity,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				chosenPresentMode,
				true,
				oldSwapchain
		);
		
		return device.createSwapchainKHR(swapchainCreateInfo);
	}
	
	Surface::Surface(Surface &&other) noexcept
	: m_Context(other.m_Context),
	  m_Handle(other.m_Handle),
	  m_PresentQueueIndex(other.m_PresentQueueIndex),
	  m_Extent(other.m_Extent),
	  m_Format(other.m_Format),
	  m_ColorSpace(other.m_ColorSpace) {
		other.m_Context = nullptr;
		other.m_Handle = nullptr;
	}
	
	Surface &Surface::operator=(Surface &&other) noexcept {
		m_Context = other.m_Context;
		m_Handle = other.m_Handle;
		m_PresentQueueIndex = other.m_PresentQueueIndex;
		m_Extent = other.m_Extent;
		m_Format = other.m_Format;
		m_ColorSpace = other.m_ColorSpace;
		
		other.m_Context = nullptr;
		other.m_Handle = nullptr;
		return *this;
	}
	
	Surface::~Surface() {
		// needs to be destroyed by creator
	}
	
	Surface Surface::create(const Window &window, const Context &context) {
		const vk::Instance& instance = context.getInstance();
		const vk::PhysicalDevice& physicalDevice = context.getPhysicalDevice();
		
		uint32_t presentQueueIndex = 0;
		
		vk::SurfaceKHR surfaceHandle;
		if (!createVulkanSurface(window.getWindow(), instance, physicalDevice, surfaceHandle))
			surfaceHandle = nullptr;
		else
			presentQueueIndex = QueueManager::checkSurfaceSupport(physicalDevice, surfaceHandle);
		
		const vk::Extent2D extent = chooseExtent(physicalDevice, surfaceHandle, window);
		const vk::SurfaceFormatKHR format = chooseSurfaceFormat(physicalDevice, surfaceHandle);
		
		return { context, surfaceHandle, presentQueueIndex, extent, format.format, format.colorSpace };
	}
	
	vk::SurfaceKHR Surface::getSurface() const {
		return m_Handle;
	}
	
	uint32_t Surface::getPresentQueueIndex() const {
		return m_PresentQueueIndex;
	}
	
	const vk::Extent2D& Surface::getExtent() const {
		return m_Extent;
	}
	
	vk::Format Surface::getFormat() const {
		return m_Format;
	}
	
	vk::ColorSpaceKHR Surface::getColorSpace() const {
		return m_ColorSpace;
	}

}
