#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vkcv {

    class Window final {
    private:
        explicit Window(GLFWwindow *window);

        GLFWwindow *m_window;

    public:
        static Window create(const char *windowTitle, int width = -1, int height = -1, bool resizable = false);

        [[nodiscard]]
        bool isWindowOpen() const;

        static void pollEvents();

        [[nodiscard]]
        GLFWwindow *getWindow() const;

        [[nodiscard]]
        int getWidth() const;

        [[nodiscard]]
        int getHeight() const;

        Window &operator=(const Window &other) = delete;

        Window &operator=(Window &&other) = default;

        virtual ~Window();

    };
}