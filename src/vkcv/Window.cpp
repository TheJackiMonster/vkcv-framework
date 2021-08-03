/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.cpp
 * @brief Window class to handle a basic rendering surface and input
 */

#include <vector>
#include <GLFW/glfw3.h>

#include "vkcv/Window.hpp"

namespace vkcv {
	
	void Window_onMouseButtonEvent(GLFWwindow *callbackWindow, int button, int action, int mods) {
		auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

		if (window != nullptr) {
			window->e_mouseButton(button, action, mods);
		}
	}

	void Window_onMouseMoveEvent(GLFWwindow *callbackWindow, double x, double y) {
		auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

		if (window != nullptr) {
			window->e_mouseMove(x, y);
		}
	}

	void Window_onMouseScrollEvent(GLFWwindow *callbackWindow, double xoffset, double yoffset) {
		auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

		if (window != nullptr) {
			window->e_mouseScroll(xoffset, yoffset);
		}
	}

	void Window_onResize(GLFWwindow *callbackWindow, int width, int height) {
		auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

		if (window != nullptr) {
			window->e_resize(width, height);
		}
	}

	void Window_onKeyEvent(GLFWwindow *callbackWindow, int key, int scancode, int action, int mods) {
		auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));

		if (window != nullptr) {
			window->e_key(key, scancode, action, mods);
		}
	}
 
	void Window_onCharEvent(GLFWwindow *callbackWindow, unsigned int c) {
		auto window = static_cast<Window *>(glfwGetWindowUserPointer(callbackWindow));
	
		if (window != nullptr) {
			window->e_char(c);
		}
	}
	
	static std::vector<GLFWwindow*> s_Windows;

	void Window_onGamepadEvent(int gamepadIndex) {
		size_t activeWindowIndex = std::find_if(
				s_Windows.begin(),
				s_Windows.end(),
				[](GLFWwindow* window){return glfwGetWindowAttrib(window, GLFW_FOCUSED);}
				) - s_Windows.begin();
	
		// fixes index getting out of bounds (e.g. if there is no focused window)
		activeWindowIndex *= (activeWindowIndex < s_Windows.size());
  
		auto window = static_cast<Window *>(glfwGetWindowUserPointer(s_Windows[activeWindowIndex]));

		if (window != nullptr) {
			window->e_gamepad(gamepadIndex);
		}
	}
	
	static GLFWwindow* createGLFWWindow(const char *windowTitle, int width, int height, bool resizable) {
		if(s_Windows.empty()) {
			glfwInit();
		}
	
		width = std::max(width, 1);
		height = std::max(height, 1);
	
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
  
		GLFWwindow *window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
	
		if (window) {
			s_Windows.push_back(window);
		}
  
		return window;
	}
	
	static void bindGLFWWindow(GLFWwindow *windowHandle, Window* window) {
		if (!windowHandle) {
			return;
		}
		
		glfwSetWindowUserPointer(windowHandle, window);
	
		// combine Callbacks with Events
		glfwSetMouseButtonCallback(windowHandle, Window_onMouseButtonEvent);
		glfwSetCursorPosCallback(windowHandle, Window_onMouseMoveEvent);
		glfwSetWindowSizeCallback(windowHandle, Window_onResize);
		glfwSetKeyCallback(windowHandle, Window_onKeyEvent);
		glfwSetScrollCallback(windowHandle, Window_onMouseScrollEvent);
		glfwSetCharCallback(windowHandle, Window_onCharEvent);
	}
	
	Window::Window() :
	m_title(),
	m_resizable(false),
	m_window(nullptr),
	e_mouseButton(true),
	e_mouseMove(true),
	e_mouseScroll(true),
	e_resize(true),
	e_key(true),
	e_char(true),
	e_gamepad(true)
	{}

	Window::Window(const char* title, int width, int height, bool resizable) :
    m_title(title),
    m_resizable(resizable),
    m_window(createGLFWWindow(title, width, height, resizable)),
	e_mouseButton(true),
	e_mouseMove(true),
	e_mouseScroll(true),
	e_resize(true),
	e_key(true),
	e_char(true),
	e_gamepad(true)
    {
		bindGLFWWindow(m_window, this);
    }

    Window::~Window() {
        Window::e_mouseButton.unlock();
        Window::e_mouseMove.unlock();
        Window::e_mouseScroll.unlock();
        Window::e_resize.unlock();
        Window::e_key.unlock();
        Window::e_char.unlock();
        Window::e_gamepad.unlock();
        
        if (m_window) {
        	s_Windows.erase(std::find(s_Windows.begin(), s_Windows.end(), m_window));
        	glfwDestroyWindow(m_window);
        }

        if(s_Windows.empty()) {
            glfwTerminate();
        }
    }
    
    Window::Window(const Window &other) :
    m_title(other.getTitle()),
    m_resizable(other.isResizable()),
    m_window(createGLFWWindow(
    		other.getTitle().c_str(),
    		other.getWidth(),
    		other.getHeight(),
    		other.isResizable()
	)),
    e_mouseButton(true),
    e_mouseMove(true),
    e_mouseScroll(true),
    e_resize(true),
    e_key(true),
    e_char(true),
    e_gamepad(true)
    {
		bindGLFWWindow(m_window, this);
    }
	
	Window &Window::operator=(const Window &other) {
		if (m_window) {
			s_Windows.erase(std::find(s_Windows.begin(), s_Windows.end(), m_window));
			glfwDestroyWindow(m_window);
		}
		
		m_title = other.getTitle();
		m_resizable = other.isResizable();
		m_window = createGLFWWindow(
				m_title.c_str(),
				other.getWidth(),
				other.getHeight(),
				m_resizable
		);
		
		bindGLFWWindow(m_window, this);
    	
    	return *this;
    }

    void Window::pollEvents() {

    	for (auto glfwWindow : s_Windows) {
			auto window = static_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));
			
			window->e_mouseButton.unlock();
			window->e_mouseMove.unlock();
			window->e_mouseScroll.unlock();
			window->e_resize.unlock();
			window->e_key.unlock();
			window->e_char.unlock();
			window->e_gamepad.unlock();
    	}

        glfwPollEvents();
    	
    	for (int gamepadIndex = GLFW_JOYSTICK_1; gamepadIndex <= GLFW_JOYSTICK_LAST; gamepadIndex++) {
    		if (glfwJoystickPresent(gamepadIndex)) {
				Window_onGamepadEvent(gamepadIndex);
			}
		}
	
		for (auto glfwWindow : s_Windows) {
			auto window = static_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));
		
			window->e_mouseButton.lock();
			window->e_mouseMove.lock();
			window->e_mouseScroll.lock();
			window->e_resize.lock();
			window->e_key.lock();
			window->e_char.lock();
			window->e_gamepad.lock();
		}
    }

    bool Window::isOpen() const {
		if (!m_window) {
			return false;
		}
		
        return !glfwWindowShouldClose(m_window);
    }
    
    const std::string& Window::getTitle() const {
    	return m_title;
    }

    int Window::getWidth() const {
        int width = 0;
        
        if (m_window) {
        	glfwGetWindowSize(m_window, &width, nullptr);
        }
        
        return std::max(width, 1);
    }

    int Window::getHeight() const {
        int height = 0;
        
        if (m_window) {
        	glfwGetWindowSize(m_window, nullptr, &height);
        }
        
        return std::max(height, 1);
    }
	
	bool Window::isResizable() const {
		return m_resizable;
	}

    GLFWwindow* Window::getWindow() const {
        return m_window;
    }
    
    void Window::getFramebufferSize(int &width, int &height) const {
		if (m_window) {
			glfwGetFramebufferSize(m_window, &width, &height);
		} else {
			width = 0;
			height = 0;
		}
    }
    
}
