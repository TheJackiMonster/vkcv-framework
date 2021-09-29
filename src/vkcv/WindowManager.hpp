#pragma once

#include <memory>
#include <vector>
#include <GLFW/glfw3.h>

#include "vkcv/Window.hpp"
#include "vkcv/Handles.hpp"
#include "SwapchainManager.hpp"

namespace vkcv {
	class Context;

	class SwapchainManager;

	class WindowManager {
		friend class Core;

	private:
		/**
		 * vector of all managed windows
		 */
		std::vector<Window*> m_windows;

		/**
		 * destroys a specific window by a given id
		 * @param id of the window to be destroyed
		 */
		void destroyWindowById(uint64_t id);

	public:
		WindowManager() noexcept;

		/**
		 * destroys every window
		 */
		~WindowManager() noexcept;

		WindowManager(WindowManager &&other) = delete;

		WindowManager(const WindowManager &other) = delete;

		WindowManager &operator=(WindowManager &&other) = delete;

		WindowManager &operator=(const WindowManager &other) = delete;

		/**
		 * creates a window and returns it's  handle
		 * @param swapchainManager for swapchain creation
		 * @param applicationName name of the window
		 * @param windowWidth
		 * @param windowHeight
		 * @param resizeable if the window is resizable
		 * @return window handle
		 */
		WindowHandle createWindow(SwapchainManager &swapchainManager, const char *applicationName, uint32_t windowWidth,
								  uint32_t windowHeight,
								  bool resizeable);

		/**
		 * @param handle of the window to get
		 * @return the reference of the window
		 */
		[[nodiscard]]
		Window &getWindow(const WindowHandle handle) const;

	};
}