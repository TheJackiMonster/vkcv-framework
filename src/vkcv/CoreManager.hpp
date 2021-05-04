#pragma once

#include <GLFW/glfw3.h>
#include <stdexcept>

namespace vkcv {

    /**
     * initializes glfw once and increases the counter
     */
    void initGLFW();

    /**
     * terminates glfw once, if it was initialized or decreases the counter
     */
    void terminateGLFW();
}
