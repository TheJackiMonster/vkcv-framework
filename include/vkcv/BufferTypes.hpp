#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/BufferTypes.hpp
 * @brief Enum classes to specify attributes of buffers.
 */

namespace vkcv {

	/**
	 * @brief Enum class to specify types of buffers.
	 */
	enum class BufferType {
		INDEX,
		VERTEX,
		UNIFORM,
		STORAGE,
		STAGING,
		INDIRECT,
		SHADER_BINDING,
		ACCELERATION_STRUCTURE_INPUT,
		ACCELERATION_STRUCTURE_STORAGE,

		UNKNOWN
	};

	/**
	 * @brief Enum class to specify types of buffer memory.
	 */
	enum class BufferMemoryType {
		DEVICE_LOCAL,
		HOST_VISIBLE,

		UNKNOWN
	};

} // namespace vkcv
