#pragma once

#include <iostream>

namespace vkcv {
	
	enum class LogLevel {
		INFO,
		WARNING,
		ERROR
	};
	
	constexpr auto getLogOutput(LogLevel level) {
		switch (level) {
			case LogLevel::INFO:
				return stdout;
			default:
				return stderr;
		}
	}
	
	constexpr const char* getLogName(LogLevel level) {
		switch (level) {
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

#define vkcv_log(level, ...) { \
  char output_message [1024];           \
                                        \
  sprintf(                              \
    output_message,                     \
    __VA_ARGS__                         \
  );                                    \
                                        \
  std::fprintf(                         \
    getLogOutput(level),                \
    "[%s]: %s [%s, line %d: %s]\n",     \
  	vkcv::getLogName(level),            \
    output_message,                     \
    __FILE__,                           \
    __LINE__,                           \
    __PRETTY_FUNCTION__                 \
  );                                    \
}

#else
#define vkcv_log(level, ...) {}
#endif

}
