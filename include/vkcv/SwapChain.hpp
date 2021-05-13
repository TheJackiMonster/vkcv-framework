#pragma once
#include "vulkan/vulkan.hpp"
#include "Context.hpp"
#include "vkcv/Window.hpp"
#include <iostream>


// glfw is not initialized in this class because ist must be sure that there exists a context first
// glfw is already initialized by the context or the window class

namespace vkcv {
    class SwapChain final {
    private:

        vk::SurfaceKHR m_surface;
        const vkcv::Context& m_context;
		vk::SwapchainKHR m_swapchain;
		vk::SurfaceFormatKHR m_format;
		
        SwapChain(vk::SurfaceKHR surface, const vkcv::Context &context, vk::SwapchainKHR swapchain, vk::SurfaceFormatKHR format);

    public:
        // bin mir grade unsicher wegen der Mehrfachinstanziierung der Klasse
        // es muessen ja oefter mal neue erstellt werden, aber diese existieren ja nicht gleichzeitig, oder?
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
