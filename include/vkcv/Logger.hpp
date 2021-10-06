#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Logger.hpp
 * @brief Logging macro function to print line of code specific information.
 */

#include <cstdio>

namespace vkcv {
	
	/**
	 * Enum class to specify the level of logging.
	 */
	enum class LogLevel {
		RAW_INFO,
		INFO,
		WARNING,
		ERROR
	};
	
	/**
	 * Return the fitting output stream to print messages of a given level of logging.
	 * @param level Level of logging
	 * @return Output stream (stdout or stderr)
	 */
	constexpr auto getLogOutput(LogLevel level) {
		switch (level) {
			case LogLevel::RAW_INFO:
			case LogLevel::INFO:
				return stdout;
			default:
				return stderr;
		}
	}
	
	/**
	 * Return the fitting identifier for messages of a given level of logging.
	 * @param level Level of logging
	 * @return Identifier of the given level of logging
	 */
	constexpr const char* getLogName(LogLevel level) {
		switch (level) {
			case LogLevel::RAW_INFO:
			case LogLevel::INFO:
				return "INFO";
			case LogLevel::WARNING:
				return "WARNING";
			case LogLevel::ERROR:
				return "ERROR";
			default:
				return "UNKNOWN";
		}
	}
	
#ifndef NDEBUG
#ifndef VKCV_DEBUG_MESSAGE_LEN
#define VKCV_DEBUG_MESSAGE_LEN 1024
#endif

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

/**
 * Macro-function to log formatting messages with a specific level of logging.
 * @param level Level of logging
 */
#define vkcv_log(level, ...) {             \
  char output_message [                    \
    VKCV_DEBUG_MESSAGE_LEN                 \
  ];                                       \
  snprintf(                                \
    output_message,                        \
    VKCV_DEBUG_MESSAGE_LEN,                \
    __VA_ARGS__                            \
  );                                       \
  auto output = getLogOutput(level);       \
  if (level != vkcv::LogLevel::RAW_INFO) { \
    fprintf(                               \
      output,                              \
      "[%s]: %s [%s, line %d: %s]\n",      \
      vkcv::getLogName(level),             \
      output_message,                      \
      __FILE__,                            \
      __LINE__,                            \
      __PRETTY_FUNCTION__                  \
    );                                     \
  } else {                                 \
    fprintf(                               \
      output,                              \
      "[%s]: %s\n",                        \
      vkcv::getLogName(level),             \
      output_message                       \
    );                                     \
  }                                        \
  fflush(output);                          \
}

#else
/**
 * Macro-function to log formatting messages with a specific level of logging.
 * @param level Level of logging
 */
#define vkcv_log(level, ...) {}
#endif

}
