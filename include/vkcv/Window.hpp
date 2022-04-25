#pragma once
/**
 * @authors Tobias Frisch, Sebastian Gaida, Vanessa Karolek, Artur Wasmut
 * @file vkcv/Window.hpp
 * @brief Class to represent and manage a window with its input.
 */

#ifndef NOMINMAX
#define NOMINMAX 1
#endif

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
		bool m_shouldClose;
		GLFWwindow *m_window;
		SwapchainHandle m_swapchainHandle;
		event_handle<int, int> m_resizeHandle;

    public:
    	/**
    	 * @brief Constructor of an uninitialized #Window
    	 */
    	Window();
    	
    	/**
    	 * @brief Constructor of a #Window with an optional width,
    	 * height and resizable attribute.
    	 *
         * @param[in] title title of the window
         * @param[in] width width of the window (optional)
         * @param[in] height height of the window (optional)
         * @param[in] resizable resize ability of the window (optional)
         */
    	explicit Window(const char* title, int width = -1, int height = -1, bool resizable = false);
	
		/**
		* @brief Copy-constructor of a #Window
		*
		* @param other Other instance of #Window
		*/
		Window(const Window& other) = delete;
	
		/**
		* @brief Copy-operator of a #Window
		*
		* @param other Other instance of #Window
		* @return Reference to itself
		*/
		Window &operator=(const Window &other) = delete;
        
        /**
         * @brief Checks if the window is still open, or the close event was called.
         * TODO: This function should be changed/removed later on
         *
         * @return True, if the window is still open, else false
         */
        [[nodiscard]]
        bool isOpen() const;

		/**
		 * @brief Gets the currently focused window and returns it
		 * TODO: only accessible to WindowManager
		 *
		 * @return Current window in focus
		 */
		static Window& getFocusedWindow();
		
		/**
		 * @brief Checks if any GLFWWindows are open
		 *
		 * @return True, if any window is open, else false
		 */
		static bool hasOpenWindow();

        /**
         * @brief Polls all events on the GLFWwindow
         */
        static void pollEvents();
		
		/**
		 * @brief Returns the required extensions to use GLFW windows with Vulkan.
		 *
		 * @return Required surface extensions
		 */
		static const std::vector<std::string>& getExtensions();
		
        event< int, int, int> e_mouseButton;
        event< double, double > e_mouseMove;
        event< double, double > e_mouseScroll;
        event< int, int > e_resize;
        event< int, int, int, int > e_key;
        event< unsigned int > e_char;
        event< int > e_gamepad;

        /**
         * @brief Returns the GLFW window handle.
         *
         * @return GLFW window handle
         */
        [[nodiscard]]
        GLFWwindow *getWindow() const;
        
        /**
         * @brief Returns the title of the window.
         *
         * @return Window title
         */
        [[nodiscard]]
        const std::string& getTitle() const;

        /**
         * @brief Returns the width of the window.
         *
         * @return Window width
         */
        [[nodiscard]]
        int getWidth() const;

        /**
         * @brief Returns the height of the window.
         *
         * @return Window height
         */
        [[nodiscard]]
        int getHeight() const;
        
        /**
         * @brief Returns whether the window is resizable or not.
         *
         * @return True, if the window is resizable, else false
         */
        [[nodiscard]]
        bool isResizable() const;

        /**
         * Destructor of #Window, terminates GLFW
         */
        virtual ~Window();

        /**
         * Requests the windows framebuffer size
         *
         * @param[out] width
         * @param[out] height
         */
        void getFramebufferSize(int& width, int& height) const;

        /**
         * @brief Retruns the andle of the swapchain in use by the window.
         *
         * @return Swapchain handle
         */
        SwapchainHandle getSwapchainHandle() const;
    };

}
