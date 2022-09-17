
#include "vkcv/File.hpp"

#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "vkcv/Logger.hpp"

namespace vkcv {

	std::filesystem::path generateTemporaryFilePath() {
		std::filesystem::path tmp = generateTemporaryDirectoryPath();

		if (std::filesystem::is_directory(tmp)) {
			return std::filesystem::path(tmp.string() + "W"); // add W for Wambo
		} else {
			return tmp;
		}
	}

	std::filesystem::path generateTemporaryDirectoryPath() {
		std::error_code code;
		auto tmp = std::filesystem::temp_directory_path(code);

		if (tmp.empty()) {
			tmp = std::filesystem::current_path();
		}

		char name [16] = "vkcv_tmp_XXXXXX";

#ifdef _WIN32
		int err = _mktemp_s(name, 16);

		if (err != 0) {
			vkcv_log(LogLevel::ERROR, "Temporary file path could not be generated");
			return "";
		}
#else
		int fd = mkstemp(name); // creates a file locally

		if (fd == -1) {
			vkcv_log(LogLevel::ERROR, "Temporary file path could not be generated");
			return "";
		}

		close(fd);
		remove(name); // removes the local file again
#endif

		return tmp / name;
	}

} // namespace vkcv
