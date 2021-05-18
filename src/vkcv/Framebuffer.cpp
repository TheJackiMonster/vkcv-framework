#include "Framebuffer.hpp"

namespace vkcv {
	vk::Framebuffer createFramebuffer(const vk::Device device, const vk::RenderPass renderpass, 
		const int width, const int height, const vk::ImageView imageView) {
		const vk::FramebufferCreateFlags flags = {};
		const uint32_t attachmentCount = 1;	// TODO: proper value
		const vk::FramebufferCreateInfo createInfo(flags, renderpass, attachmentCount, &imageView, width, height, 1);
		return device.createFramebuffer(createInfo, nullptr, {});
	}
}