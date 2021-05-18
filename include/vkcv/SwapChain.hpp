#pragma once
#include "vulkan/vulkan.hpp"
#include "Context.hpp"
#include "vkcv/Window.hpp"

namespace vkcv {
    class SwapChain final {
    private:

        vk::SurfaceKHR m_surface;
        vk::SwapchainKHR m_swapchain;
        vk::SurfaceFormatKHR m_format;

        /**
         * Constructor of a SwapChain object
         * glfw is not initialized in this class because ist must be sure that there exists a context first
         * glfw is already initialized by the window class
         * @param surface used by the swapchain
         * @param swapchain to show images in the window
         * @param format
         */
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

        /**
         * gets the current surface object
         * @return current surface
         */
        [[nodiscard]]
        vk::SurfaceKHR getSurface();
        /**
         * gets the current surface format
         * @return gets the surface format
         */
        [[nodiscard]]
        vk::SurfaceFormatKHR getSurfaceFormat();

        /**
         * creates a swap chain object out of the given window and the given context
         * @param window a wrapper that represents a glfw window
         * @param context of the application
         * @return returns an object of swapChain
         */
        static SwapChain create(const Window &window, const Context &context);

        /**
         * Destructor of SwapChain
         */
        virtual ~SwapChain();
    };

}
