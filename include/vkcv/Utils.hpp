#pragma once
#include <string>

namespace utils {

    extern std::string g_binaryDir;
    
    /**
     * @brief 
     * 
     * @param path The relative path
     * @return std::string The absolute path
     */
    std::string absolutePath(std::string path);

    /**
     * @brief Set the Binary Dir object
     * 
     * @param argv0 
     */
    void setBinaryDir(const char* argv0);
}
