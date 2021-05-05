/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.cpp
 * @brief Window class to handle a basic rendering surface and input
 */

#include "Window.hpp"
#include "CoreManager.hpp"

namespace vkcv {

    Window::Window(GLFWwindow *window)
            : m_window(window) {
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        vkcv::terminateGLFW();
    }

    Window Window::create(const char *windowTitle, int width, int height, bool resizable) {
        vkcv::initGLFW();
        width = std::max(width, 1);
        height = std::max(height, 1);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
        GLFWwindow *window;
        window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
        return Window(window);
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

    int Window::getWidth() const {
        int width;
        glfwGetWindowSize(m_window, &width, nullptr);
        return width;
    }

    int Window::getHeight() const {
        int height;
        glfwGetWindowSize(m_window, nullptr, &height);
        return height;
    }
}