#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv{
	vk::Framebuffer createFramebuffer(const vk::Device device, const vk::RenderPass renderpass,
		const int width, const int height, const vk::ImageView imageView);
}