/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.cpp
 * @brief Window class to handle a basic rendering surface and input
 */

#include "Window.hpp"
#include "CoreManager.hpp"


namespace vkcv {

    Window::Window(GLFWwindow *window, const vkcv::SwapChain *swapChain)
            : m_window(window), m_swapChain(swapChain){
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        vkcv::terminateGLFW();
    }

    Window Window::create(const vkcv::Context& context ,const char *windowTitle, int width, int height, bool resizable) {
        vkcv::initGLFW();
        width = std::max(width, 1);
        height = std::max(height, 1);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        GLFWwindow *window;
        window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);

       const vkcv::SwapChain swapChain = vkcv::SwapChain::create(
                window,
                &context);

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