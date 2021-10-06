#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch
 * @file vkcv/ImageConfig.hpp
 * @brief File to provide functions supporting the use of multisampling.
 */

#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	enum class Multisampling { None, MSAA2X, MSAA4X, MSAA8X };

	vk::SampleCountFlagBits msaaToVkSampleCountFlag(Multisampling msaa);
	uint32_t                msaaToSampleCount(Multisampling msaa);
	
}
