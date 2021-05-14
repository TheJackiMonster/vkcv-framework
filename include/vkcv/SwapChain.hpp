#pragma once
#include "vulkan/vulkan.hpp"
#include "Context.hpp"
#include "vkcv/Window.hpp"
#include <iostream>

namespace vkcv {
    class SwapChain final {
    private:

        vk::SurfaceKHR m_surface;
		vk::SwapchainKHR m_swapchain;
		vk::SurfaceFormatKHR m_format;
		
        SwapChain(vk::SurfaceKHR surface, vk::SwapchainKHR swapchain, vk::SurfaceFormatKHR format);

    public:
        SwapChain(const SwapChain &other) = default;
        SwapChain(SwapChain &&other) = default;

        /**
         * @return The swapchain linked with the #SwapChain class
         * @note The reference to our Swapchain variable is needed for the recreation step
         */
        [[nodiscard]]
        vk::SwapchainKHR getSwapchain();

        [[nodiscard]]
        vk::SurfaceKHR getSurface();

        [[nodiscard]]
        vk::SurfaceFormatKHR getSurfaceFormat();

        static SwapChain create(const Window &window, const Context &context);
        virtual ~SwapChain();
    };

}
