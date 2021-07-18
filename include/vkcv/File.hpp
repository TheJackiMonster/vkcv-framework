#pragma once

#include <filesystem>

namespace vkcv {
	
	std::filesystem::path generateTemporaryFilePath();
	
	std::filesystem::path generateTemporaryDirectoryPath();
	
}
