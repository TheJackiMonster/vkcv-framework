#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/File.hpp
 * @brief Functions to handle generating temporary file paths.
 */

#include <filesystem>

namespace vkcv {
	
	/**
	 * Generate a new temporary file path and return it.
	 * @return A unique path for a temporary file
	 */
	std::filesystem::path generateTemporaryFilePath();
	
	/**
	 * Generate a new temporary directory path and return it.
	 * @return A unique path for a temporary directory
	 */
	std::filesystem::path generateTemporaryDirectoryPath();
	
}
