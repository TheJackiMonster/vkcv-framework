#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv {
	void transitionImageLayoutImmediate(const vk::CommandBuffer cmdBuffer, const VkImage image,
		const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout);
}