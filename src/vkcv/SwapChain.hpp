#pragma once
#include "vulkan/vulkan.hpp"
#include "Context.hpp"
#include "Window.hpp"


// glfw is not initialized in this class because ist must be sure that there exists a context first
// glfw is already initialized by the context or the window class

namespace vkcv {

    class SwapChain final {
    private:
        vk::SurfaceKHR m_surface;

        SwapChain(vk::SurfaceKHR);

    public:
        // bin mir grade unsicher wegen der Mehrfachinstanziierung der Klasse
        // es muessen ja oefter mal neue erstellt werden, aber diese existieren ja nicht gleichzeitig, oder?
        SwapChain(const SwapChain &other) = delete;
        SwapChain(SwapChain &&other) = default;
        static SwapChain create(GLFWwindow *window, const vk::Instance& instance,const vk::PhysicalDevice& physicalDevice,const vk::Device& device);
        static void createSurface(GLFWwindow *window, vk::SurfaceKHR& surface, const vk::Instance& instance, const vk::PhysicalDevice& physicalDevice);

    };

}