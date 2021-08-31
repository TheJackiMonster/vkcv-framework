#pragma once

#define NOMINMAX
#include <algorithm>
#include <string>

#include "Event.hpp"
#include "Handles.hpp"

struct GLFWwindow;

namespace vkcv {

    class Window {
		friend class WindowManager;
		friend class SwapchainManager;
		
	private:
    	std::string m_title;
    	bool m_resizable;
		GLFWwindow *m_window;
		SwapchainHandle m_swapchainHandle;
		bool m_shouldClose;

    public:
    	/**
    	 * creates an uninitialized #Window
    	 */
    	Window();
    	
    	/**
    	 * creates a #Window with the parameters
    	 *
         * @param[in] title title of the window
         * @param[in] width width of the window (optional)
         * @param[in] height height of the window (optional)
         * @param[in] resizable resize ability of the window (optional)
         */
    	explicit Window(const char* title, int width = -1, int height = -1, bool resizable = false);
	
		/**
		* Copy-constructor of #Window
		*
		* @param other Other instance of #Window
		*/
		Window(const Window& other) = delete;
	
		/**
		* Copy-operator of #Window
		*
		* @param other Other instance of #Window
		* @return Reference to itself
		*/
		Window &operator=(const Window &other) = delete;
        
        /**
         * checks if the window is still open, or the close event was called
         * This function should be changed/removed later on
         * @return bool if the window is still open
         */
        [[nodiscard]]
        bool isOpen() const;

		/**
		 * gets the currently focused window and returns it
		 * only accessible to WindowManager
		 * @return
		 */
		static Window& getFocusedWindow();
		
		/**
		 *
		 * @return
		 */
		static bool hasOpenWindow();

        /**
         * polls all events on the GLFWwindow
         */
        static void pollEvents();
		
		/**
		 *
		 * @return
		 */
		static const std::vector<std::string>& getExtensions();

        /**
         * basic events of the window
         */
        event< int, int, int> e_mouseButton;
        event< double, double > e_mouseMove;
        event< double, double > e_mouseScroll;
        event< int, int > e_resize;
        event< int, int, int, int > e_key;
        event< unsigned int > e_char;
        event< int > e_gamepad;

        /**
         * returns the current window
         * @return window handle
         */
        [[nodiscard]]
        GLFWwindow *getWindow() const;
        
        /**
         * gets the window title
         * @return string with window title
         */
        [[nodiscard]]
        const std::string& getTitle() const;

        /**
         * gets the window width
         * @return int with window width
         */
        [[nodiscard]]
        int getWidth() const;

        /**
         * gets the window height
         * @return int with window height
         */
        [[nodiscard]]
        int getHeight() const;
        
        /**
         * is the window resizable
         * @return bool with window resizable
         */
        [[nodiscard]]
        bool isResizable() const;

        /**
         * Destructor of #Window, terminates GLFW
         */
        virtual ~Window();

        /**
         * destroys the window
         */
		//void destroyWindow();

        /**
         * gets the windows framebuffer size
         * @param width
         * @param height
         */
        void getFramebufferSize(int& width, int& height) const;

        /**
         * gets the SwapchainHandle corresponding to the swapchain of the window
         * @return
         */
        SwapchainHandle getSwapchainHandle() const;
    };

}
