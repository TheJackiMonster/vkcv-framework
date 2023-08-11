#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Logger.hpp
 * @brief Logging macro function to print line of code specific information.
 */

#include <bits/chrono.h>
#include <cstdio>
#include <exception>
#include <chrono>

namespace vkcv {

	/**
	 * @brief Enum class to specify the level of logging.
	 */
	enum class LogLevel {
		TIME,
		RAW_INFO,
		INFO,
		WARNING,
		ERROR
	};

	/**
	 * @brief Return the fitting output stream to print messages
	 * of a given level of logging.
	 *
	 * @param[in] level Level of logging
	 * @return Output stream (stdout or stderr)
	 */
	constexpr auto getLogOutput(LogLevel level) {
		switch (level) {
		case LogLevel::TIME:
		case LogLevel::RAW_INFO:
		case LogLevel::INFO:
			return stdout;
		default:
			return stderr;
		}
	}

	/**
	 * @brief Returns the fitting identifier for messages of
	 * a given level of logging.
	 *
	 * @param[in] level Level of logging
	 * @return Identifier of the given level of logging
	 */
	constexpr const char* getLogName(LogLevel level) {
		switch (level) {
		case LogLevel::TIME:
			return "TIME";
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

	inline unsigned long getLogTime() {
		const auto time_point = std::chrono::high_resolution_clock::now();
		static auto last_time_point = time_point;

		const auto duration = (time_point - last_time_point);
		last_time_point = time_point;

		return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	}

#ifndef NDEBUG
#ifndef VKCV_DEBUG_MESSAGE_LEN
#define VKCV_DEBUG_MESSAGE_LEN 1024
#endif

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

/**
 * @brief Macro-function to log formatting messages with
 * a specific level of logging.
 *
 * @param[in] level Level of logging
 */
#define vkcv_log(level, ...)                                                         \
	{                                                                                \
		char output_message [VKCV_DEBUG_MESSAGE_LEN];                                \
		snprintf(output_message, VKCV_DEBUG_MESSAGE_LEN, __VA_ARGS__);               \
		auto output = getLogOutput(level);                                           \
		if (level == vkcv::LogLevel::TIME) {                                         \
			fprintf(output, "[%s]: %s (%lums)\n", vkcv::getLogName(level),           \
					output_message, vkcv::getLogTime());                             \
		} else                                                                       \
		if (level != vkcv::LogLevel::RAW_INFO) {                                     \
			fprintf(output, "[%s]: %s [%s, line %d: %s]\n", vkcv::getLogName(level), \
					output_message, __FILE__, __LINE__, __PRETTY_FUNCTION__);        \
		} else {                                                                     \
			fprintf(output, "[%s]: %s\n", vkcv::getLogName(level), output_message);  \
		}                                                                            \
		fflush(output);                                                              \
	}

#else
/**
 * @brief Macro-function to log formatting messages with
 * a specific level of logging.
 *
 * @param[in] level Level of logging
 */
#define vkcv_log(level, ...) \
	{}
#endif

/**
 * @brief Macro-function to log the message of any error
 * or an exception.
 *
 * @param[in] error Error or exception
 */
#define vkcv_log_error(error) \
	{ vkcv_log(LogLevel::ERROR, "%s", (error).what()); }

/**
 * @brief Macro-function to throw and log any error or
 * an exception.
 *
 * @param[in] error Error or exception
 */
#define vkcv_log_throw(error)               \
	{                                       \
		try {                               \
			throw error;                    \
		} catch (const std::exception &e) { \
			vkcv_log_error(e);              \
			throw;                          \
		}                                   \
	}

/**
 * @brief Macro-function to throw and log an error
 * with its custom message.
 *
 * @param[in] message Error message
 */
#define vkcv_log_throw_error(message) \
	{ vkcv_log_throw(std::runtime_error(message)); }

} // namespace vkcv
