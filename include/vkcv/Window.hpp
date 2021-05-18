#pragma once
/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.hpp
 * @brief Window class to handle a basic rendering surface and input
 */
#include <GLFW/glfw3.h>

#define NOMINMAX
#include <algorithm>

namespace vkcv {
    class Window final {
    private:
        GLFWwindow *m_window;


        /**
         *
         * @param GLFWwindow of the class
         */
        explicit Window(GLFWwindow *window);

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