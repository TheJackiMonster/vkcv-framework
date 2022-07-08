#pragma once

#include <memory>
#include <vector>
#include <GLFW/glfw3.h>

#include "vkcv/Window.hpp"

#include "HandleManager.hpp"
#include "SwapchainManager.hpp"

namespace vkcv {

	/**
	 * @brief Class to manage the windows of applications.
	 */
	class WindowManager : public HandleManager<Window*, WindowHandle> {
		friend class Core;

	private:
		[[nodiscard]]
		uint64_t getIdFrom(const WindowHandle& handle) const override;
		
		[[nodiscard]]
		WindowHandle createById(uint64_t id, const HandleDestroyFunction& destroy) override;
		
		/**
		 * Destroys a specific window by a given id.
		 *
		 * @param[in] id ID of the window to be destroyed
		 */
		void destroyById(uint64_t id) override;

	public:
		WindowManager() noexcept;

		/**
		 * destroys every window
		 */
		~WindowManager() noexcept override;

		/**
		 * creates a window and returns it's  handle
		 * @param swapchainManager for swapchain creation
		 * @param applicationName name of the window
		 * @param windowWidth
		 * @param windowHeight
		 * @param resizeable if the window is resizable
		 * @return window handle
		 */
		WindowHandle createWindow(SwapchainManager &swapchainManager,
								  const char *applicationName,
								  uint32_t windowWidth,
								  uint32_t windowHeight,
								  bool resizeable);

		/**
		 * @param handle of the window to get
		 * @return the reference of the window
		 */
		[[nodiscard]]
		Window &getWindow(const WindowHandle& handle) const;

	};
	
}