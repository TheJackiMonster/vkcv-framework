#ifndef VKCV_COREMANAGER_HPP
#define VKCV_COREMANAGER_HPP

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

#endif //VKCV_COREMANAGER_HPP
