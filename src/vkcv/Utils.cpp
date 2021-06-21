#include <vkcv/Utils.hpp>
#include <filesystem>

namespace utils {

    std::string g_binaryDir;

    std::string absolutePath(std::string path) {
        std::filesystem::path relative(path);
        std::filesystem::path fullpath = g_binaryDir / relative;
        return fullpath.string();
    }

    void setBinaryDir(const char* argv0) {
        std::filesystem::path binaryPath(argv0);
        std::filesystem::path binaryDir = binaryPath.parent_path();
        utils::g_binaryDir = binaryDir.string();
    }
}