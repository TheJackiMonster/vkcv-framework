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

        SwapChain(vk::SurfaceKHR surface, const vkcv::Core* core);

    public:
        // bin mir grade unsicher wegen der Mehrfachinstanziierung der Klasse
        // es muessen ja oefter mal neue erstellt werden, aber diese existieren ja nicht gleichzeitig, oder?
        SwapChain(const SwapChain &other) = delete;
        SwapChain(SwapChain &&other) = default;

        static SwapChain create(GLFWwindow *window, const vkcv::Core* core);

        virtual ~SwapChain();
    };

}