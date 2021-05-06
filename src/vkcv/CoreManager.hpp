#pragma once
/**
 * @authors Sebastian Gaida
 * @file src/vkcv/CoreManager.hpp
 * @brief Handling of global states regarding dependencies
 */

#include <GLFW/glfw3.h>
#include "vulkan/vulkan.hpp"
#include <stdexcept>
#include <vector>

namespace vkcv {

    /**
     * initializes glfw once and increases the counter
     */
    void initGLFW();

    /**
     * terminates glfw once, if it was initialized or decreases the counter
     */
    void terminateGLFW();

    /**
     * gets the window width
     * @param window glfwWindow
     * @return int with window width
     */
    int getWidth(GLFWwindow *window);

    /**
     * gets the window height
     * @param window glfwWindow
     * @return int with window height
     */
    int getHeight(GLFWwindow *window);
}
