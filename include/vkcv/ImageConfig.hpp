#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv {
	enum class Multisampling { None, MSAA2X, MSAA4X, MSAA8X };

	vk::SampleCountFlagBits msaaToVkSampleCountFlag(Multisampling msaa);
}
