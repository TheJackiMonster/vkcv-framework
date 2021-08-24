#pragma once

#include <vector>
#include <GLFW/glfw3.h>

#include "vkcv/WindowManager.hpp"
#include "vkcv/Swapchain.hpp"
#include "vkcv/Handles.hpp"

namespace vkcv {
	class Core;

	class SwapchainManager {
		friend class Core;

		friend class WindowManager;

	private:

		Context *m_context;
		void destroySwapchainById(uint64_t id);

	public:
		SwapchainManager() noexcept;

		~SwapchainManager() noexcept;

		SwapchainManager(SwapchainManager &&other) = delete;

		SwapchainManager(const SwapchainManager &other) = delete;

		SwapchainManager &operator=(SwapchainManager &&other) = delete;

		SwapchainManager &operator=(const SwapchainManager &other) = delete;

		SwapchainHandle createSwapchain(Window &window);

		[[nodiscard]]
		Swapchain &getSwapchain(const SwapchainHandle handle) const;

		void signalRecreation(const SwapchainHandle handle);

		std::vector<vk::Image> getSwapchainImages(const SwapchainHandle handle);

		std::vector<vk::ImageView> createSwapchainImageViews(SwapchainHandle handle);
	};
}