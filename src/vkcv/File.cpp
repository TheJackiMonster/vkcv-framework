
#include "vkcv/File.hpp"

#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <fstream>

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
	
	bool writeContentToFile(const std::filesystem::path &path,
							const Vector<char>& content) {
		std::ofstream file (path.string(), std::ios::out);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)",
							 path.string().c_str());
			return false;
		}
		
		file.seekp(0);
		file.write(content.data(), static_cast<std::streamsize>(content.size()));
		file.close();
		
		return true;
	}
	
	bool writeBinaryToFile(const std::filesystem::path &path,
						   const Vector<uint32_t>& binary) {
		std::ofstream file (path.string(), std::ios::out);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)",
							 path.string().c_str());
			return false;
		}
		
		file.seekp(0);
		file.write(
				reinterpret_cast<const char*>(binary.data()),
				static_cast<std::streamsize>(binary.size() * sizeof(uint32_t))
		);
		file.close();
		
		return true;
	}
	
	bool writeTextToFile(const std::filesystem::path &path,
						 const std::string& text) {
		std::ofstream file (path.string(), std::ios::out);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)",
							 path.string().c_str());
			return false;
		}
		
		file.seekp(0);
		file.write(text.c_str(), static_cast<std::streamsize>(text.length()));
		file.close();
		
		return true;
	}
	
	bool readContentFromFile(const std::filesystem::path &path,
							 Vector<char>& content) {
		std::ifstream file (path.string(), std::ios::ate);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)",
							 path.string().c_str());
			return false;
		}
		
		const std::streamsize fileSize = file.tellg();
		content.resize(fileSize);
		
		file.seekg(0);
		file.read(content.data(), fileSize);
		file.close();
		
		return true;
	}
	
	bool readBinaryFromFile(const std::filesystem::path &path,
							Vector<uint32_t>& binary) {
		std::ifstream file (path.string(), std::ios::ate);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)",
							 path.string().c_str());
			return false;
		}
		
		const std::streamsize fileSize = file.tellg();
		
		if (fileSize % sizeof(uint32_t) != 0) {
			vkcv_log(LogLevel::ERROR, "The file is not a valid binary: %s",
							 path.string().c_str());
			return false;
		}
		
		binary.resize(fileSize / sizeof(uint32_t));
		
		file.seekg(0);
		file.read(
				reinterpret_cast<char*>(binary.data()),
				static_cast<std::streamsize>(binary.size() * sizeof(uint32_t))
		);
		file.close();
		
		return true;
	}
	
	bool readTextFromFile(const std::filesystem::path &path,
						  std::string& text) {
		std::ifstream file (path.string(), std::ios::ate);
		
		if (!file.is_open()) {
			vkcv_log(LogLevel::ERROR, "The file could not be opened (%s)",
							 path.string().c_str());
			return false;
		}
		
		const std::streamsize fileSize = file.tellg();
		Vector<char> buffer (fileSize);
		buffer.resize(fileSize);
		
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		
		text = std::string(buffer.data(), buffer.size());
		return true;
	}

} // namespace vkcv
