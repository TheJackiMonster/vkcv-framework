/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.cpp
 * @brief Window class to handle a basic rendering surface and input
 */

#include <GLFW/glfw3.h>
#include "vkcv/Window.hpp"

namespace vkcv {

	static std::vector<GLFWwindow*> s_Windows;

    Window::Window(GLFWwindow *window)
            : m_window(window) {
		glfwSetWindowUserPointer(m_window, this);
	
		this->e_mouseButton.lock();
		this->e_mouseMove.lock();
		this->e_resize.lock();
		this->e_key.lock();
		this->e_mouseScroll.lock();
	
		// combine Callbacks with Events
		glfwSetMouseButtonCallback(m_window, Window::onMouseButtonEvent);
		glfwSetCursorPosCallback(m_window, Window::onMouseMoveEvent);
		glfwSetWindowSizeCallback(m_window, Window::onResize);
		glfwSetKeyCallback(m_window, Window::onKeyEvent);
		glfwSetScrollCallback(m_window, Window::onMouseScrollEvent);
		glfwSetCharCallback(m_window, Window::onCharEvent);
	
		glfwSetJoystickCallback(Window::onGamepadConnection);
		glfwSetJoystickUserPointer(GLFW_JOYSTICK_1, this);
    }

    Window::~Window() {
		s_Windows.erase(std::find(s_Windows.begin(), s_Windows.end(), m_window));
        glfwDestroyWindow(m_window);

        if(s_Windows.empty()) {
            glfwTerminate();
        }
    }

    Window Window::create( const char *windowTitle, int width, int height, bool resizable) {
		if(s_Windows.empty()) {
			glfwInit();
		}
	
		width = std::max(width, 1);
		height = std::max(height, 1);
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
		GLFWwindow *window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
	
		s_Windows.push_back(window);
	
		return Window(window);
    }

    void Window::pollEvents() {
		onGamepadEvent(GLFW_JOYSTICK_1);
    	
    	for (auto glfwWindow : s_Windows) {
			auto window = static_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));
			
			window->e_mouseButton.unlock();
			window->e_mouseMove.unlock();
			window->e_resize.unlock();
			window->e_key.unlock();
			window->e_mouseScroll.unlock();
    	}

        glfwPollEvents();
	
		for (auto glfwWindow : s_Windows) {
			auto window = static_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));
		
			window->e_mouseButton.lock();
			window->e_mouseMove.lock();
			window->e_resize.lock();
			window->e_key.lock();
			window->e_mouseScroll.lock();
		}
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

    void Window::onGamepadConnection(int gamepadIndex, int gamepadEvent) {
        if (gamepadEvent == GLFW_CONNECTED) {
            auto window = static_cast<Window *>(glfwGetWindowUserPointer(s_Windows[0]));    // todo check for correct window

            if (window != nullptr) {
                glfwSetJoystickUserPointer(gamepadIndex, window);
            }
        }
    }

    void Window::onGamepadEvent(int gamepadIndex) {
        auto window = static_cast<Window *>(glfwGetJoystickUserPointer(gamepadIndex));

        if ( window != nullptr && glfwJoystickPresent(gamepadIndex)) {
            window->e_gamepad(gamepadIndex);
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
