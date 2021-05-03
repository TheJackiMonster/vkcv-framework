#pragma once

#include <GLFW/glfw3.h>
#include <stdexcept>

namespace vkcv {

    /**
     * initialize GLFW if not initialized
     */
    void initGLFW();
    /**
     * terminate GLFW
     */
    void terminateGLFW();
}
