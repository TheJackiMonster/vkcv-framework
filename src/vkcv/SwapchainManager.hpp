#pragma once

#include <vector>
#include <GLFW/glfw3.h>

#include "WindowManager.hpp"
#include "vkcv/Swapchain.hpp"
#include "vkcv/Handles.hpp"

namespace vkcv {
	class Core;

	class SwapchainManager {
		friend class Core;

		friend class WindowManager;

	private:
		std::vector<Swapchain> m_swapchains;

		Context *m_context;

		void destroySwapchainById(uint64_t id);

	public:
		SwapchainManager() noexcept;

		~SwapchainManager() noexcept;

		SwapchainManager(SwapchainManager &&other) = delete;

		SwapchainManager(const SwapchainManager &other) = delete;

		SwapchainManager &operator=(SwapchainManager &&other) = delete;

		SwapchainManager &operator=(const SwapchainManager &other) = delete;

		/**
		 * creates a swapchain and returns the handle
		 * @param window of the to  creatable window
		 * @return the swapchainHandle of the created swapchain
		 */
		SwapchainHandle createSwapchain(Window &window);

		/**
		 * @param handle of the swapchain to get
		 * @return the reference of the swapchain
		 */
		[[nodiscard]]
		Swapchain &getSwapchain(const SwapchainHandle& handle);

		/**
		 * sets  the recreation  flag fot the swapchain
		 * @param handle of the swapchain that should be recreated
		 */
		void signalRecreation(const SwapchainHandle& handle);

		/**
		 * gets the swapchain images
		 * @param handle of the swapchain
		 * @return a vector of the swapchain images
		 */
		std::vector<vk::Image> getSwapchainImages(const SwapchainHandle& handle);

		/**
		 * creates the swapchain imageViews for the swapchain
		 * @param handle of the swapchain which ImageViews should be created
		 * @return a ov ImageViews of the swapchain
		 */
		std::vector<vk::ImageView> createSwapchainImageViews(SwapchainHandle& handle);
	};
}