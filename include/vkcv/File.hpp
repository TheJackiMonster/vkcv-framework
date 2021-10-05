#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/File.hpp
 * @brief Functions to handle generating temporary file paths.
 */

#include <filesystem>

namespace vkcv {
	
	std::filesystem::path generateTemporaryFilePath();
	
	std::filesystem::path generateTemporaryDirectoryPath();
	
}
