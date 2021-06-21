#include <vkcv/Utils.hpp>
#include <boost/dll.hpp>

namespace utils {
    
    std::string absolutePath(std::string path) {
        boost::filesystem::path root = boost::dll::program_location().parent_path();
        boost::filesystem::path relative(path);
        boost::filesystem::path fullpath = root / relative;
        return fullpath.string();
    }
}