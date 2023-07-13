#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/File.hpp
 * @brief Functions to handle generating temporary file paths.
 */

#include <filesystem>

#include "Container.hpp"

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
	
	/**
	 * @brief Write content data from a vector to a file at
	 * a given path.
	 *
	 * @param[in] path Path of file
	 * @param[in] content Custom data vector
	 * @return True on success, false otherwise
	 */
	bool writeContentToFile(const std::filesystem::path &path,
							const Vector<char>& content);
	
	/**
	 * @brief Write binary data from a vector to a file at
	 * a given path.
	 *
	 * @param[in] path Path of file
	 * @param[in] binary Custom binary vector
	 * @return True on success, false otherwise
	 */
	bool writeBinaryToFile(const std::filesystem::path &path,
						   const Vector<uint32_t>& binary);
	
	/**
	 * @brief Write text to a file at a given path.
	 *
	 * @param[in] path Path of file
	 * @param[in] text Custom text as string
	 * @return True on success, false otherwise
	 */
	bool writeTextToFile(const std::filesystem::path &path,
						 const std::string& text);
	
	/**
	 * @brief Read content data from a file at a given path
	 * into a vector.
	 *
	 * @param[in] path Path of file
	 * @param[in] content Custom data vector reference
	 * @return True on success, false otherwise
	 */
	bool readContentFromFile(const std::filesystem::path &path,
							 Vector<char>& content);
	
	/**
	 * @brief Read binary data from a file at a given path
	 * into a vector.
	 *
	 * @param[in] path Path of file
	 * @param[in] content Custom binary vector reference
	 * @return True on success, false otherwise
	 */
	bool readBinaryFromFile(const std::filesystem::path &path,
							Vector<uint32_t>& binary);
	
	/**
	 * @brief Read text from a file at a given path.
	 *
	 * @param[in] path Path of file
	 * @param[in] text Custom text as string reference
	 * @return True on success, false otherwise
	 */
	bool readTextFromFile(const std::filesystem::path &path,
						  std::string& text);

} // namespace vkcv
