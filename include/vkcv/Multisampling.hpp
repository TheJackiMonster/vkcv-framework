#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch
 * @file vkcv/ImageConfig.hpp
 * @brief File to provide functions supporting the use of multisampling.
 */

#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	enum class Multisampling {
		None,
		MSAA2X,
		MSAA4X,
		MSAA8X
	};

	/**
	 * @brief Returns the sample count flag bits of a given
	 * multi-sample anti-aliasing mode.
	 *
	 * @param[in] msaa MSAA mode
	 * @return Sample count flag bits
	 */
	vk::SampleCountFlagBits msaaToSampleCountFlagBits(Multisampling msaa);
	
	/**
	 * @brief Returns the amount of samples of a given
	 * multi-sample anti-aliasing mode.
	 *
	 * @param msaa MSAA mode
	 * @return Number of samples
	 */
	uint32_t msaaToSampleCount(Multisampling msaa);
	
}
