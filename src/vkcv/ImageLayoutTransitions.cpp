#include "ImageLayoutTransitions.hpp"

namespace vkcv {
	void transitionImageLayoutImmediate(const vk::CommandBuffer cmdBuffer, const vk::Image image,
		const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout) {

		// TODO: proper src and dst masks
		const vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands;
		const vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands;
		const vk::DependencyFlags dependecyFlags = {};

		// TODO: proper src and dst masks
		const vk::AccessFlags srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		const vk::AccessFlags dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		// TODO: proper aspect flags
		const vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor;

		const vk::ImageSubresourceRange subresourceRange(aspectFlags, 0, 1, 0, 1);
		vk::ImageMemoryBarrier imageBarrier(srcAccessMask, dstAccessMask, oldLayout, newLayout, 0, 0, image, subresourceRange);

		cmdBuffer.pipelineBarrier(srcStageMask, dstStageMask, dependecyFlags, 0, nullptr, 0, nullptr, 1, &imageBarrier, {});
	}
}