#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Sampler.hpp
 * @brief Enums for different sampler attributes.
 */

namespace vkcv {

    /**
     * @brief Enum class to specify a samplers type to filter during access.
     */
	enum class SamplerFilterType {
		NEAREST = 1,
		LINEAR = 2
	};

    /**
     * @brief Enum class to specify a samplers mode to access mipmaps.
     */
	enum class SamplerMipmapMode {
		NEAREST = 1,
		LINEAR = 2
	};

    /**
     * @brief Enum class to specify a samplers mode to access via address space.
     */
	enum class SamplerAddressMode {
		REPEAT = 1,
		MIRRORED_REPEAT = 2,
		CLAMP_TO_EDGE = 3,
		MIRROR_CLAMP_TO_EDGE = 4,
		CLAMP_TO_BORDER = 5
	};

    /**
     * @brief Enum class to specify a samplers color beyond a textures border.
     */
	enum class SamplerBorderColor {
		INT_ZERO_OPAQUE = 1,
		INT_ZERO_TRANSPARENT = 2,
		
		FLOAT_ZERO_OPAQUE = 3,
		FLOAT_ZERO_TRANSPARENT = 4,
		
		INT_ONE_OPAQUE = 5,
		FLOAT_ONE_OPAQUE = 6
	};

}
