#include "WindowManager.hpp"

namespace vkcv {
	
	WindowManager::WindowManager() noexcept {
	}

	WindowManager::~WindowManager() noexcept {
		for (uint64_t id = 0; id < m_windows.size(); id++) {
			destroyWindowById(id);
		}
		
		m_windows.clear();
	}

	WindowHandle WindowManager::createWindow(
			SwapchainManager &swapchainManager,
			const char *applicationName,
			uint32_t windowWidth,
			uint32_t windowHeight,
			bool resizeable) {
		const uint64_t id = m_windows.size();

		auto window = new Window(applicationName, windowWidth, windowHeight, resizeable);

		SwapchainHandle swapchainHandle = swapchainManager.createSwapchain(*window);

		if (resizeable) {
			const event_handle<int, int> &resizeHandle = window->e_resize.add([&](int width, int height) {
				swapchainManager.signalRecreation(swapchainHandle);
			});
			window->m_resizeHandle = resizeHandle;
		}

		m_windows.push_back(window);
		return WindowHandle(id, [&](uint64_t id) { destroyWindowById(id); });
	}

	Window &WindowManager::getWindow(const WindowHandle handle) const {
		return *m_windows[handle.getId()];
	}

	void WindowManager::destroyWindowById(uint64_t id) {

		if (id >= m_windows.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid id");
			return;
		}

		if (m_windows[id] != nullptr) {
			delete m_windows[id];
			m_windows[id] = nullptr;
		}
	}
}