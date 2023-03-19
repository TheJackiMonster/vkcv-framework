#include "WindowManager.hpp"

namespace vkcv {

	uint64_t WindowManager::getIdFrom(const WindowHandle &handle) const {
		return handle.getId();
	}

	WindowHandle WindowManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return WindowHandle(id, destroy);
	}

	void WindowManager::destroyById(uint64_t id) {
		auto &window = getById(id);

		if (window) {
			delete window;
			window = nullptr;
		}
	}

	WindowManager::WindowManager() noexcept : HandleManager<Window*, WindowHandle>() {}

	WindowManager::~WindowManager() noexcept {
		clear();
	}

	WindowHandle WindowManager::createWindow(SwapchainManager &swapchainManager,
											 const std::string &applicationName,
											 uint32_t windowWidth, uint32_t windowHeight,
											 bool resizeable) {
		auto window = new Window(applicationName, static_cast<int>(windowWidth),
								 static_cast<int>(windowHeight), resizeable);

		SwapchainHandle swapchainHandle = swapchainManager.createSwapchain(*window);
		
		if (!swapchainHandle) {
			delete window;
			return {};
		}

		if (resizeable) {
			const event_handle<int, int> &resizeHandle =
				window->e_resize.add([&, handle = swapchainHandle](int width, int height) {
					// copy handle because it would run out of scope and be invalid
					swapchainManager.signalRecreation(handle);
				});

			window->m_resizeHandle = resizeHandle;
		}

		return add(window);
	}

	Window &WindowManager::getWindow(const WindowHandle &handle) const {
		return *(*this) [handle];
	}

	Vector<WindowHandle> WindowManager::getWindowHandles() const {
		Vector<WindowHandle> handles;

		for (size_t id = 0; id < getCount(); id++) {
			if (getById(id)->isOpen()) {
				handles.push_back(WindowHandle(id));
			}
		}

		return handles;
	}

} // namespace vkcv