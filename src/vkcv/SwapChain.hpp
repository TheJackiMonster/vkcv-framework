#pragma once
#include "vulkan/vulkan.hpp"
#include "Context.hpp"
#include <GLFW/glfw3.h>
#include <iostream>


// glfw is not initialized in this class because ist must be sure that there exists a context first
// glfw is already initialized by the context or the window class

namespace vkcv {
    class SwapChain final {
    private:

        vk::SurfaceKHR m_surface;
        const vkcv::Context* m_context;

        SwapChain(vk::SurfaceKHR surface, const vkcv::Context* context);

        static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

        static vk::PresentModeKHR choosePresentMode(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

        static vk::Extent2D chooseSwapExtent(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, GLFWwindow* window );

    public:
        // bin mir grade unsicher wegen der Mehrfachinstanziierung der Klasse
        // es muessen ja oefter mal neue erstellt werden, aber diese existieren ja nicht gleichzeitig, oder?
        SwapChain(const SwapChain &other) = delete;
        SwapChain(SwapChain &&other) = default;

        static SwapChain create(GLFWwindow *window, const vkcv::Context* Context);
        static vk::SurfaceKHR createSurface(GLFWwindow *window, const vk::Instance& instance, const vk::PhysicalDevice& physicalDevice);

        virtual ~SwapChain();
    };

}