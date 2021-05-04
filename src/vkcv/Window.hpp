#pragma once
/**
 * @authors Sebastian Gaida
 * @file src/vkcv/Window.hpp
 * @brief Window class to handle a basic rendering surface and input
 */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vkcv {

    class Window final {
    private:
        explicit Window(GLFWwindow *window);

        GLFWwindow *m_window;

    public:
        /**
         * creates a GLFWwindow with the parameters in the function
         * @param windowTitle
         * @param width
         * @param height
         * @param resizable
         * @return Window class
         */
        static Window create(const char *windowTitle, int width = -1, int height = -1, bool resizable = false);

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
         * gets the current window width
         * @return int with window width
         */
        [[nodiscard]]
        int getWidth() const;

        /**
         * gets the current window height
         * @return int with window height
         */
        [[nodiscard]]
        int getHeight() const;

        Window &operator=(const Window &other) = delete;

        Window &operator=(Window &&other) = default;

        virtual ~Window();

    };
}