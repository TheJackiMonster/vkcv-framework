/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.cpp
 * @brief Window class to handle a basic rendering surface and input
 */

#include "vkcv/Window.hpp"


namespace vkcv {

    static uint32_t s_WindowCount = 0;

    Window::Window(GLFWwindow *window, const vkcv::SwapChain *swapChain)
            : m_window(window), m_swapChain(swapChain) {
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        s_WindowCount--;

        terminateGLFW();
    }

    Window Window::create(const vkcv::Core& core, const char *windowTitle, int width, int height, bool resizable) {
        initGLFW();

        s_WindowCount++;

        width = std::max(width, 1);
        height = std::max(height, 1);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        GLFWwindow *window;
        window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);

       const vkcv::SwapChain swapChain = vkcv::SwapChain::create(
                window,
                &core);

        return Window(window, &swapChain);
    }

    bool Window::isWindowOpen() const {
        return !glfwWindowShouldClose(m_window);
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    GLFWwindow *Window::getWindow() const {
        return m_window;
    }
}
