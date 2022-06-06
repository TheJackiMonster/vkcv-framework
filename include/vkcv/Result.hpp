#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Result.hpp
 * @brief Enum to represent result values of function which can fail.
 */

namespace vkcv {

    /**
     * @brief Enum class to specify the result of a function call.
     */
	enum class Result {
		SUCCESS = 0,
		ERROR = 1
	};
	
}
