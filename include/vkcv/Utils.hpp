#pragma once
#include <string>

namespace utils {
    
    /**
     * @brief Given a relative path to the executable, the absolute path is returned
     * 
     * @param path The relative path
     * @return std::string The absolute path
     */
    std::string absolutePath(std::string path);
}
