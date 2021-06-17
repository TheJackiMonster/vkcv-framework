/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.cpp
 * @brief Window class to handle a basic rendering surface and input
 */

#include <GLFW/glfw3.h>

#include "vkcv/Window.hpp"

namespace vkcv {

    static uint32_t s_WindowCount = 0;

    Window::Window(GLFWwindow *window)
            : m_window(window) {
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        s_WindowCount--;

        if(s_WindowCount == 0) {
            glfwTerminate();
        }
    }
	
    GLFWwindow* Window::createGLFWWindow(const char *windowTitle, int width, int height, bool resizable) {
		if(s_WindowCount == 0) {
			glfwInit();
		}
	
		s_WindowCount++;
	
		width = std::max(width, 1);
		height = std::max(height, 1);
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
		
		return glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
    }

    Window Window::create( const char *windowTitle, int width, int height, bool resizable) {
        return Window(createGLFWWindow(windowTitle, width, height, resizable));
    }

    void Window::initEvents() {
        glfwSetWindowUserPointer(m_window, this);

        // combine Callbacks with Events
        glfwSetMouseButtonCallback(m_window, Window::onMouseButtonEvent);

        glfwSetCursorPosCallback(m_window, Window::onMouseMoveEvent);

        glfwSetWindowSizeCallback(m_window, Window::onResize);

        glfwSetKeyCallback(m_window, Window::onKeyEvent);

        glfwSetScrollCallback(m_window, Window::onMouseScrollEvent);
	
		glfwSetCharCallback(m_window, Window::onCharEvent);
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    void Window::onMouseButtonEvent(GLFWwindow *callbackWindow, int button, int action, int mods) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

        if (window != nullptr) {
            window->e_mouseButton(button, action, mods);
        }
    }

    void Window::onMouseMoveEvent(GLFWwindow *callbackWindow, double x, double y) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

        if (window != nullptr) {
            window->e_mouseMove(x, y);
        }
    }

    void Window::onMouseScrollEvent(GLFWwindow *callbackWindow, double xoffset, double yoffset) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

        if (window != nullptr) {
            window->e_mouseScroll(xoffset, yoffset);
        }
    }

    void Window::onResize(GLFWwindow *callbackWindow, int width, int height) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

        if (window != nullptr) {
            window->e_resize(width, height);
        }
    }

    void Window::onKeyEvent(GLFWwindow *callbackWindow, int key, int scancode, int action, int mods) {
        auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

        if (window != nullptr) {
            window->e_key(key, scancode, action, mods);
        }
    }
    
    void Window::onCharEvent(GLFWwindow *callbackWindow, unsigned int c) {
		auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));
	
		if (window != nullptr) {
			window->e_char(c);
		}
    }

    bool Window::isWindowOpen() const {
        return !glfwWindowShouldClose(m_window);
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

    GLFWwindow *Window::getWindow() const {
        return m_window;
    }
}