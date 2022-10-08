#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/File.hpp
 * @brief Functions to handle generating temporary file paths.
 */

#include <filesystem>

namespace vkcv {

	/**
	 * @brief Generate a new temporary file path and return it.
	 *
	 * @return A unique path for a temporary file
	 */
	std::filesystem::path generateTemporaryFilePath();

	/**
	 * @brief Generate a new temporary directory path and return it.
	 *
	 * @return A unique path for a temporary directory
	 */
	std::filesystem::path generateTemporaryDirectoryPath();

} // namespace vkcv
