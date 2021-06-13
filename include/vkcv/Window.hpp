#pragma once
/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.hpp
 * @brief Window class to handle a basic rendering surface and input
 */

#define NOMINMAX
#include <algorithm>
#include "Event.hpp"

struct GLFWwindow;

namespace vkcv {

    class Window final {
    private:
        GLFWwindow *m_window;

        /**
         *
         * @param GLFWwindow of the class
         */
        explicit Window(GLFWwindow *window);

        /**
         * mouse callback for moving the mouse on the screen
         * @param[in] window The window that received the event.
         * @param[in] xpos The new cursor x-coordinate, relative to the left edge of the content area.
         * @param[in] ypos The new cursor y-coordinate, relative to the top edge of the content area.
         */
        static void onMouseMoveEvent(GLFWwindow *window, double x, double y);

        /**
         * mouseButton callback for mouse buttons
         * @param[in] button The [mouse button](@ref buttons) that was pressed or released.
         * @param[in] action One of `GLFW_PRESS` or `GLFW_RELEASE`.  Future releases may add more actions.
         * @param[in] mods Bit field describing which [modifier keys](@ref mods) were held down.
         */
        static void onMouseButtonEvent(GLFWwindow *callbackWindow, int button, int action, int mods);

        static void onMouseScrollEvent(GLFWwindow *callbackWindow, double xoffset, double yoffset);

        /**
         * resize callback for the resize option of the window
         * @param[in] window The window that was resized.
         * @param[in] width The new width, in screen coordinates, of the window.
         * @param[in] height The new height, in screen coordinates, of the window.
         */
        static void onResize(GLFWwindow *callbackWindow, int width, int height);

        /**
         * key callback for the pressed key
         * @param[in] window The window that received the event.
         * @param[in] key The [keyboard key](@ref keys) that was pressed or released.
         * @param[in] scancode The system-specific scancode of the key.
         * @param[in] action `GLFW_PRESS`, `GLFW_RELEASE` or `GLFW_REPEAT`.
         * @param[in] mods Bit field describing which [modifier keys](@ref mods) were held down.
         */
        static void onKeyEvent(GLFWwindow *callbackWindow, int key, int scancode, int action, int mods);

    public:
        /**
         * creates a GLFWwindow with the parameters in the function
         * @param[in] windowTitle of the window
         * @param[in] width of the window (optional)
         * @param[in] height of the window (optional)
         * @param[in] resizable resize ability of the window (optional)
         * @return Window class
         */
        static Window create( const char *windowTitle, int width = -1, int height = -1, bool resizable = false);
        /**
         * checks if the window is still open, or the close event was called
         * This function should be changed/removed later on
         * @return bool if the window is still open
         */
        [[nodiscard]]
        bool isWindowOpen() const;

        /**
         * binds windowEvents to lambda events
         */
        void initEvents();

        /**
         * polls all events on the GLFWwindow
         */
        static void pollEvents();

        /**
         * basic events of the window
         */
        event< int, int, int> e_mouseButton;
        event< double, double > e_mouseMove;
        event< double, double > e_mouseScroll;
        event< int, int > e_resize;
        event< int, int, int, int > e_key;

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
         * gets the window width
         * @param window glfwWindow
         * @return int with window width
         */
        [[nodiscard]]
        int getWidth() const;

        /**
         * gets the window height
         * @param window glfwWindow
         * @return int with window height
         */
        [[nodiscard]]
        int getHeight() const;

        /**
         * Destructor of #Window, terminates GLFW
         */
        virtual ~Window();
    };

}