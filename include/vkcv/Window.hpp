#pragma once
/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.hpp
 * @brief Window class to handle a basic rendering surface and input
 */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "SwapChain.hpp"

#define NOMINMAX
#include <algorithm>

namespace vkcv {
    class Window final {
    private:
        GLFWwindow *m_window;

        const vkcv::SwapChain* m_swapChain;

        /**
         *
         * @param GLFWwindow of the class
         */
        Window(GLFWwindow *window, const vkcv::SwapChain *swapChain);

    public:
        /**
         * creates a GLFWwindow with the parameters in the function
         * @param[in] windowTitle of the window
         * @param[in] width of the window (optional)
         * @param[in] height of the window (optional)
         * @param[in] resizable resize ability of the window (optional)
         * @return Window class
         */
        static Window create(const vkcv::Context& context, const char *windowTitle, int width = -1, int height = -1, bool resizable = false);
        /**
         * checks if the window is still open, or the close event was called
         * This function should be changed/removed later on
         * @return bool if the window is still open
         */
        [[nodiscard]]
        bool isWindowOpen() const;

        /**
         * polls all events on the GLFWwindow
         */
        static void pollEvents();

        /**
         * returns the current window
         * @return window handle
         */
        [[nodiscard]]
        GLFWwindow *getWindow() const;

        /**
         * Copy-operator of #Window is deleted!
         *
         * @param other Other instance of #Window
         * @return Reference to itself
         */
        Window &operator=(const Window &other) = delete;

        /**
         * Move-operator of #Window uses default behavior!
         *
         * @param other Other instance of #Window
         * @return Reference to itself
         */
        Window &operator=(Window &&other) = default;

        /**
         * Destructor of #Window, terminates GLFW
         */
        virtual ~Window();

    };
}