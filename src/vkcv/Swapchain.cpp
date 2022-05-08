#include <vkcv/Swapchain.hpp>
#include <utility>

#include <GLFW/glfw3.h>

namespace vkcv
{

    Swapchain::Swapchain(const Context &context,
						 const Surface &surface,
                         vk::SwapchainKHR swapchain) noexcept
						 : m_Context(&context),
						   m_Surface(surface),
						   m_Swapchain(swapchain),
						   m_RecreationRequired(false) {
	}
    
    Swapchain::Swapchain(const Swapchain &other) :
	m_Context(other.m_Context),
	m_Surface(other.m_Surface),
	m_Swapchain(other.m_Swapchain),
	m_RecreationRequired(other.m_RecreationRequired.load()) {
	}

    const vk::SwapchainKHR& Swapchain::getSwapchain() const {
        return m_Swapchain;
    }

    const Surface& Swapchain::getSurface() const {
        return m_Surface;
    }

    vk::Format Swapchain::getFormat() const{
        return m_Surface.getFormat();
    }

    Swapchain Swapchain::create(const Window &window, const Context &context) {
        Surface surface = Surface::create(window, context);

        vk::SwapchainKHR swapchain = surface.createVulkanSwapchain(
				window, nullptr
		);

        return { context, surface, swapchain };
    }
    
    bool Swapchain::shouldUpdateSwapchain() const {
    	return m_RecreationRequired;
    }
    
    void Swapchain::updateSwapchain(const Context &context, const Window &window) {
    	if (!m_RecreationRequired.exchange(false)) {
			return;
		}
    	
		vk::SwapchainKHR oldSwapchain = m_Swapchain;
	
		m_Swapchain = m_Surface.createVulkanSwapchain(
				window, oldSwapchain
		);
		
		if (!m_Swapchain) {
			signalSwapchainRecreation();
		}
		
		if (oldSwapchain) {
			context.getDevice().destroySwapchainKHR(oldSwapchain);
		}
    }

    void Swapchain::signalSwapchainRecreation() {
		m_RecreationRequired = true;
    }
    
    const vk::Extent2D& Swapchain::getExtent() const {
    	return m_Surface.getExtent();
    }

    Swapchain::~Swapchain() {
        // needs to be destroyed by creator
    }

	uint32_t Swapchain::getImageCount() const {
		uint32_t imageCount = 0;
		
		if (vk::Result::eSuccess != m_Context->getDevice().getSwapchainImagesKHR(m_Swapchain, &imageCount, nullptr))
			return 0;
		else
			return imageCount;
	}

	uint32_t Swapchain::getPresentQueueIndex() const {
		return m_Surface.getPresentQueueIndex();
	}
	
}
