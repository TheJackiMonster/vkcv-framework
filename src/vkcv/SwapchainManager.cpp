#include <vkcv/SwapchainManager.hpp>

namespace vkcv {
	static std::vector<Swapchain> m_swapchains;

	SwapchainManager::SwapchainManager() noexcept {
	}

	SwapchainManager::~SwapchainManager() noexcept {
		for (uint64_t id = 0; id < m_swapchains.size(); id++) {
			destroySwapchainById(id);
		}
		m_swapchains.clear();
	}

	[[maybe_unused]] SwapchainHandle SwapchainManager::createSwapchain(Window &window, Context &context) {
		const uint64_t id = m_swapchains.size();

		Swapchain swapchain = Swapchain::create(window, context);

		m_swapchains.push_back(swapchain);
		return SwapchainHandle(id, [&](uint64_t id) { destroySwapchainById(id); });
	}

	Swapchain &SwapchainManager::getSwapchain(const SwapchainHandle handle) const {
		return m_swapchains[handle.getId()];
	}

	void SwapchainManager::destroySwapchainById(uint64_t id) {

		if (id >= m_swapchains.size()) {
			vkcv_log(LogLevel::ERROR, "Invalid id");
			return;
		}
	}
}