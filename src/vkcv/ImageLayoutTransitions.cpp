#include "ImageLayoutTransitions.hpp"
#include "vkcv/Image.hpp"

namespace vkcv {
	vk::ImageMemoryBarrier createImageLayoutTransitionBarrier(const ImageManager::Image &image, vk::ImageLayout newLayout) {

		vk::ImageAspectFlags aspectFlags;
		if (isDepthFormat(image.m_format)) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
		}
		else {
			aspectFlags = vk::ImageAspectFlagBits::eColor;
		}

		vk::ImageSubresourceRange imageSubresourceRange(
			aspectFlags,
			0,
			image.m_viewPerMip.size(),
			0,
			image.m_layers
		);

		// TODO: precise AccessFlagBits, will require a lot of context
		return vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eMemoryWrite,
			vk::AccessFlagBits::eMemoryRead,
			image.m_layout,
			newLayout,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			image.m_handle,
			imageSubresourceRange);
	}

	vk::ImageMemoryBarrier createSwapchainImageLayoutTransitionBarrier(
		vk::Image       vulkanHandle, 
		vk::ImageLayout oldLayout, 
		vk::ImageLayout newLayout) {

		vk::ImageSubresourceRange imageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0,
			1,
			0,
			1);

		// TODO: precise AccessFlagBits, will require a lot of context
		return vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eMemoryWrite,
			vk::AccessFlagBits::eMemoryRead,
			oldLayout,
			newLayout,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			vulkanHandle,
			imageSubresourceRange);
	}

	void recordImageBarrier(vk::CommandBuffer cmdBuffer, vk::ImageMemoryBarrier barrier) {
		cmdBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			{},
			nullptr,
			nullptr,
			barrier);
	}
}