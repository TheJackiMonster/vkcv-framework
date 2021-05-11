#pragma once
#include "vulkan/vulkan.hpp"
#include "Core.hpp"
#include <GLFW/glfw3.h>
#include <iostream>


// glfw is not initialized in this class because ist must be sure that there exists a context first
// glfw is already initialized by the context or the window class

namespace vkcv {
    class SwapChain final {
    private:

        vk::SurfaceKHR m_surface;
        const vkcv::Core* m_core;
		vk::SwapchainKHR m_swapchain;
		
        SwapChain(vk::SurfaceKHR surface, const vkcv::Core* core, vk::SwapchainKHR swapchain);

    public:
        // bin mir grade unsicher wegen der Mehrfachinstanziierung der Klasse
        // es muessen ja oefter mal neue erstellt werden, aber diese existieren ja nicht gleichzeitig, oder?
        SwapChain(const SwapChain &other) = delete;
        SwapChain(SwapChain &&other) = default;

        /**
         * @return The swapchain linked with the #SwapChain class
         * @note The reference to our Swapchain variable is needed for the recreation step
         */
        [[nodiscard]]
        vk::SwapchainKHR getSwapchain();

        static SwapChain create(GLFWwindow *window, const vkcv::Core* core);
       
        virtual ~SwapChain();
    };

}
