#include "SwapchainManager.hpp"

namespace vkcv {

	SwapchainManager::SwapchainManager() noexcept {
	}

	SwapchainManager::~SwapchainManager() noexcept {
		for (uint64_t id = 0; id < m_swapchains.size(); id++) {
			destroySwapchainById(id);
		}
		m_swapchains.clear();
	}

	SwapchainHandle SwapchainManager::createSwapchain(Window &window) {
		const uint64_t id = m_swapchains.size();

		Swapchain swapchain = Swapchain::create(window, *m_context);

		m_swapchains.push_back(swapchain);
		SwapchainHandle swapchainHandle = SwapchainHandle(id, [&](uint64_t id) { destroySwapchainById(id); });
		window.m_swapchainHandle = swapchainHandle;
		return swapchainHandle;
	}

	Swapchain& SwapchainManager::getSwapchain(const SwapchainHandle& handle) {
		return m_swapchains[handle.getId()];
	}

	void SwapchainManager::destroySwapchainById(uint64_t id) {

		if (id >= m_swapchains.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid id");
			return;
		}
		Swapchain &swapchain = m_swapchains[id];

		if (swapchain.m_Swapchain) {
			m_context->getDevice().destroySwapchainKHR(swapchain.m_Swapchain);
			swapchain.m_Swapchain = nullptr;
		}
		
		if (swapchain.m_Surface.m_Handle) {
			m_context->getInstance().destroySurfaceKHR(swapchain.m_Surface.m_Handle);
			swapchain.m_Surface.m_Handle = nullptr;
		}
	}

	void SwapchainManager::signalRecreation(const SwapchainHandle& handle) {
		m_swapchains[handle.getId()].signalSwapchainRecreation();
	}

	std::vector<vk::Image> SwapchainManager::getSwapchainImages(const SwapchainHandle& handle) {
		return m_context->getDevice().getSwapchainImagesKHR(m_swapchains[handle.getId()].getSwapchain());
	}

	std::vector<vk::ImageView> SwapchainManager::createSwapchainImageViews(SwapchainHandle& handle) {
		std::vector<vk::Image> images = getSwapchainImages(handle);
		Swapchain &swapchain = m_swapchains[handle.getId()];

		std::vector<vk::ImageView> imageViews;
		imageViews.reserve(images.size());
		//here can be swizzled with vk::ComponentSwizzle if needed
		vk::ComponentMapping componentMapping(
				vk::ComponentSwizzle::eR,
				vk::ComponentSwizzle::eG,
				vk::ComponentSwizzle::eB,
				vk::ComponentSwizzle::eA);

		vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		for (auto image : images) {
			vk::ImageViewCreateInfo imageViewCreateInfo(
					vk::ImageViewCreateFlags(),
					image,
					vk::ImageViewType::e2D,
					swapchain.getFormat(),
					componentMapping,
					subResourceRange);

			imageViews.push_back(m_context->getDevice().createImageView(imageViewCreateInfo));
		}
		return imageViews;
	}
}