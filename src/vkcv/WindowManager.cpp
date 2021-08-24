#include "vkcv/WindowManager.hpp"
#include "vkcv/Core.hpp"

namespace vkcv {
	static std::vector<Window> m_windows;

	WindowManager::WindowManager() noexcept {
	}

	WindowManager::~WindowManager() noexcept {
		for (uint64_t id = 0; id < m_windows.size(); id++) {
			destroyWindowById(id);
		}
		m_windows.clear();
	}

	WindowHandle WindowManager::createWindow(
			Core &core,
			const char *applicationName,
			uint32_t windowWidth,
			uint32_t windowHeight,
			bool resizeable) {
		const uint64_t id = m_windows.size();

		vkcv::Window window = vkcv::Window(applicationName, windowWidth, windowHeight, resizeable);

		Swapchain swapChain = Swapchain::create(window, core.getContext());

		if (resizeable) {
			window.e_resize.add([&](int width, int height) {
				// m_swapchain.signalSwapchainRecreation(); // swapchain signal
			});
		}

		m_windows.push_back(window);
		return WindowHandle(id, [&](uint64_t id) { destroyWindowById(id); });
	}

	Window &WindowManager::getWindow(const WindowHandle handle) const {
		return m_windows[handle.getId()];
	}

	void WindowManager::destroyWindowById(uint64_t id) {

		if (id >= m_windows.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid id");
			return;
		}

		Window &window = m_windows[id];
		window.destroyWindow();
		m_windows[id] = nullptr;
	}

	void WindowManager::pollEvents() {
		Window::pollEvents();
	}

	bool WindowManager::hasOpenWindow() {
		for (auto &window : m_windows) {
			if (window.isOpen()) {
				return true;
			}
		}
		return false;
	}

	const Window WindowManager::getFocusedWindow() {
		return Window::getFocusedWindow();
	}
}