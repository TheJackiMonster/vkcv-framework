#pragma once

#include <vector>
#include <GLFW/glfw3.h>

#include "vkcv/Window.hpp"
#include "vkcv/Handles.hpp"

namespace vkcv {
	class Core;

	class WindowManager {
		friend class Core;

	private:

		void destroyWindowById(uint64_t id);

	public:
		WindowManager() noexcept;

		~WindowManager() noexcept;

		WindowManager(WindowManager &&other) = delete;

		WindowManager(const WindowManager &other) = delete;

		WindowManager &operator=(WindowManager &&other) = delete;

		WindowManager &operator=(const WindowManager &other) = delete;

		WindowHandle createWindow(Core &core, const char *applicationName, uint32_t windowWidth, uint32_t windowHeight,
								  bool resizeable);

		[[nodiscard]]
		Window &getWindow(const WindowHandle handle) const;

		/**
		 * Forwards the event poll to "vkcv/Window.hpp"
		 */
		static void pollEvents();

		static bool hasOpenWindow();

		static const Window getFocusedWindow();
	};
}