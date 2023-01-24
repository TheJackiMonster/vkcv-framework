#include "SwapchainManager.hpp"

#include <GLFW/glfw3.h>

#include "vkcv/Core.hpp"

namespace vkcv {

	uint64_t SwapchainManager::getIdFrom(const SwapchainHandle &handle) const {
		return handle.getId();
	}

	SwapchainHandle SwapchainManager::createById(uint64_t id,
												 const HandleDestroyFunction &destroy) {
		return SwapchainHandle(id, destroy);
	}

	void SwapchainManager::destroyById(uint64_t id) {
		auto &swapchain = getById(id);

		if (swapchain.m_Swapchain) {
			getCore().getContext().getDevice().destroySwapchainKHR(swapchain.m_Swapchain);
			swapchain.m_Swapchain = nullptr;
		}

		if (swapchain.m_Surface) {
			getCore().getContext().getInstance().destroySurfaceKHR(swapchain.m_Surface);
			swapchain.m_Surface = nullptr;
		}
	}

	SwapchainManager::SwapchainManager() noexcept :
		HandleManager<SwapchainEntry, SwapchainHandle>() {}

	SwapchainManager::~SwapchainManager() noexcept {
		clear();
	}

	/**
	 * @brief Creates vulkan surface and checks availability.
	 *
	 * @param[in,out] window Current window for the surface
	 * @param[in,out] instance Vulkan-Instance
	 * @param[in,out] physicalDevice Vulkan-PhysicalDevice
	 * @param[out] surface Vulkan-Surface
	 * @return Created vulkan surface
	 */
	static bool createVulkanSurface(GLFWwindow* window, const vk::Instance &instance,
									const vk::PhysicalDevice &physicalDevice,
									vk::SurfaceKHR &surface) {
		VkSurfaceKHR api_surface;

		if (glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &api_surface)
			!= VK_SUCCESS) {
			vkcv_log(LogLevel::ERROR, "Failed to create a window surface");
			return false;
		}

		vk::Bool32 surfaceSupport = false;
		surface = vk::SurfaceKHR(api_surface);

		if ((physicalDevice.getSurfaceSupportKHR(0, surface, &surfaceSupport)
			 != vk::Result::eSuccess)
			|| (!surfaceSupport)) {
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
	static vk::Extent2D chooseExtent(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface,
									 const Window &window) {
		int fb_width, fb_height;
		window.getFramebufferSize(fb_width, fb_height);

		VkExtent2D extent2D = { static_cast<uint32_t>(fb_width), static_cast<uint32_t>(fb_height) };

		vk::SurfaceCapabilitiesKHR surfaceCapabilities;
		if (physicalDevice.getSurfaceCapabilitiesKHR(surface, &surfaceCapabilities)
			!= vk::Result::eSuccess) {
			vkcv_log(LogLevel::WARNING, "The capabilities of the surface can not be retrieved");

			extent2D.width = std::max(MIN_SURFACE_SIZE, extent2D.width);
			extent2D.height = std::max(MIN_SURFACE_SIZE, extent2D.height);
		} else {
			extent2D.width =
				std::max(surfaceCapabilities.minImageExtent.width,
						 std::min(surfaceCapabilities.maxImageExtent.width, extent2D.width));
			extent2D.height =
				std::max(surfaceCapabilities.minImageExtent.height,
						 std::min(surfaceCapabilities.maxImageExtent.height, extent2D.height));
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
	static vk::SurfaceFormatKHR chooseSurfaceFormat(vk::PhysicalDevice physicalDevice,
													vk::SurfaceKHR surface) {
		std::vector<vk::SurfaceFormatKHR> availableFormats =
			physicalDevice.getSurfaceFormatsKHR(surface);

		for (const auto &availableFormat : availableFormats) {
			if (availableFormat.format == vk::Format::eB8G8R8A8Unorm
				&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return availableFormat;
			}
		}

		return availableFormats [0];
	}

	/**
	 * @brief Returns vk::PresentModeKHR::eMailbox if available or
	 * vk::PresentModeKHR::eFifo otherwise
	 *
	 * @param physicalDevice Vulkan-PhysicalDevice
	 * @param surface Vulkan-Surface of the swapchain
	 * @return Available PresentationMode
	 */
	static vk::PresentModeKHR choosePresentMode(vk::PhysicalDevice physicalDevice,
												vk::SurfaceKHR surface) {
		std::vector<vk::PresentModeKHR> availablePresentModes =
			physicalDevice.getSurfacePresentModesKHR(surface);

		for (const auto &availablePresentMode : availablePresentModes) {
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
	static uint32_t chooseImageCount(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
		vk::SurfaceCapabilitiesKHR surfaceCapabilities =
			physicalDevice.getSurfaceCapabilitiesKHR(surface);

		// minImageCount should always be at least 2; set to 3 for triple buffering
		uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

		// check if requested image count is supported
		if (surfaceCapabilities.maxImageCount > 0
			&& imageCount > surfaceCapabilities.maxImageCount) {
			imageCount = surfaceCapabilities.maxImageCount;
		}

		return imageCount;
	}

	static bool createVulkanSwapchain(const Context &context, const Window &window,
									  SwapchainEntry &entry) {
		if (!context.getFeatureManager().isExtensionActive(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
			vkcv_log(LogLevel::WARNING, "Extension required to create a swapchain: '%s'",
					 VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			return false;
		}
		
		const vk::PhysicalDevice &physicalDevice = context.getPhysicalDevice();
		const vk::Device &device = context.getDevice();

		entry.m_Extent = chooseExtent(physicalDevice, entry.m_Surface, window);

		if ((entry.m_Extent.width < MIN_SURFACE_SIZE)
			|| (entry.m_Extent.height < MIN_SURFACE_SIZE)) {
			return false;
		}

		vk::SurfaceFormatKHR chosenSurfaceFormat =
			chooseSurfaceFormat(physicalDevice, entry.m_Surface);
		vk::PresentModeKHR chosenPresentMode = choosePresentMode(physicalDevice, entry.m_Surface);
		uint32_t chosenImageCount = chooseImageCount(physicalDevice, entry.m_Surface);

		entry.m_Format = chosenSurfaceFormat.format;
		entry.m_ColorSpace = chosenSurfaceFormat.colorSpace;

		vk::SwapchainCreateInfoKHR swapchainCreateInfo(
			vk::SwapchainCreateFlagsKHR(), entry.m_Surface, chosenImageCount, entry.m_Format,
			entry.m_ColorSpace, entry.m_Extent, 1,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage,
			vk::SharingMode::eExclusive, 0, nullptr, vk::SurfaceTransformFlagBitsKHR::eIdentity,
			vk::CompositeAlphaFlagBitsKHR::eOpaque, chosenPresentMode, true, entry.m_Swapchain);

		entry.m_Swapchain = device.createSwapchainKHR(swapchainCreateInfo);
		return true;
	}

	SwapchainHandle SwapchainManager::createSwapchain(Window &window) {
		const vk::Instance &instance = getCore().getContext().getInstance();
		const vk::PhysicalDevice &physicalDevice = getCore().getContext().getPhysicalDevice();

		vk::SurfaceKHR surfaceHandle;
		if (!createVulkanSurface(window.getWindow(), instance, physicalDevice, surfaceHandle)) {
			return {};
		}

		uint32_t presentQueueIndex =
			QueueManager::checkSurfaceSupport(physicalDevice, surfaceHandle);

		const vk::Extent2D extent = chooseExtent(physicalDevice, surfaceHandle, window);
		const vk::SurfaceFormatKHR format = chooseSurfaceFormat(physicalDevice, surfaceHandle);

		SwapchainEntry entry { nullptr,          false,

							   surfaceHandle,    presentQueueIndex,
							   extent,           format.format,
							   format.colorSpace };

		if (!createVulkanSwapchain(getCore().getContext(), window, entry)) {
			instance.destroySurfaceKHR(surfaceHandle);
			return {};
		}

		window.m_swapchainHandle = add(entry);
		return window.m_swapchainHandle;
	}

	SwapchainEntry &SwapchainManager::getSwapchain(const SwapchainHandle &handle) {
		return (*this) [handle];
	}

	bool SwapchainManager::shouldUpdateSwapchain(const SwapchainHandle &handle) const {
		return (*this) [handle].m_RecreationRequired;
	}

	void SwapchainManager::updateSwapchain(const SwapchainHandle &handle, const Window &window) {
		auto &swapchain = (*this) [handle];

		if (!swapchain.m_RecreationRequired) {
			return;
		} else {
			swapchain.m_RecreationRequired = false;
		}

		vk::SwapchainKHR oldSwapchain = swapchain.m_Swapchain;

		if (createVulkanSwapchain(getCore().getContext(), window, swapchain)) {
			if (oldSwapchain) {
				getCore().getContext().getDevice().destroySwapchainKHR(oldSwapchain);
			}
		} else {
			signalRecreation(handle);
		}
	}

	void SwapchainManager::signalRecreation(const SwapchainHandle &handle) {
		(*this) [handle].m_RecreationRequired = true;
	}

	vk::Format SwapchainManager::getFormat(const SwapchainHandle &handle) const {
		return (*this) [handle].m_Format;
	}

	uint32_t SwapchainManager::getImageCount(const SwapchainHandle &handle) const {
		auto &swapchain = (*this) [handle];

		uint32_t imageCount;
		if (vk::Result::eSuccess
			!= getCore().getContext().getDevice().getSwapchainImagesKHR(swapchain.m_Swapchain,
																		&imageCount, nullptr)) {
			return 0;
		} else {
			return imageCount;
		}
	}

	const vk::Extent2D &SwapchainManager::getExtent(const SwapchainHandle &handle) const {
		return (*this) [handle].m_Extent;
	}

	uint32_t SwapchainManager::getPresentQueueIndex(const SwapchainHandle &handle) const {
		return (*this) [handle].m_PresentQueueIndex;
	}

	vk::ColorSpaceKHR SwapchainManager::getSurfaceColorSpace(const SwapchainHandle &handle) const {
		return (*this) [handle].m_ColorSpace;
	}

	std::vector<vk::Image>
	SwapchainManager::getSwapchainImages(const SwapchainHandle &handle) const {
		return getCore().getContext().getDevice().getSwapchainImagesKHR(
			(*this) [handle].m_Swapchain);
	}

	std::vector<vk::ImageView>
	SwapchainManager::createSwapchainImageViews(SwapchainHandle &handle) {
		std::vector<vk::Image> images = getSwapchainImages(handle);
		auto &swapchain = (*this) [handle];

		std::vector<vk::ImageView> imageViews;
		imageViews.reserve(images.size());
		// here can be swizzled with vk::ComponentSwizzle if needed
		vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
											  vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);

		vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		for (auto image : images) {
			vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), image,
														vk::ImageViewType::e2D, swapchain.m_Format,
														componentMapping, subResourceRange);

			imageViews.push_back(
				getCore().getContext().getDevice().createImageView(imageViewCreateInfo));
		}

		return imageViews;
	}
} // namespace vkcv