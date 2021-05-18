#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv {
	void transitionImageLayoutImmediate(const vk::CommandBuffer cmdBuffer, const vk::Image image,
		const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout);
}