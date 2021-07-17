
#include "vkcv/File.hpp"

#include <stdlib.h>
#include <unistd.h>

namespace vkcv {
	
	std::filesystem::path generateTemporaryFilePath() {
		std::error_code code;
		auto tmp = std::filesystem::temp_directory_path(code);
		
		if (tmp.empty()) {
			tmp = std::filesystem::current_path();
		}
		
		char name [16] = "vkcv_tmp_XXXXXX";
		int fd = mkstemp(name);
		
		if (fd == -1) {
			return "";
		}
		
		close(fd);
		return tmp / name;
	}
	
}
