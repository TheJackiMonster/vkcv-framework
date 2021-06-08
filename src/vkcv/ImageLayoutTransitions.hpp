#pragma once
#include <vulkan/vulkan.hpp>
#include "ImageManager.hpp"

namespace vkcv {
	vk::ImageMemoryBarrier createImageLayoutTransitionBarrier(const ImageManager::Image& image, vk::ImageLayout newLayout);
	vk::ImageMemoryBarrier createSwapchainImageLayoutTransitionBarrier(
		vk::Image       vulkanHandle,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout);

	void recordImageBarrier(vk::CommandBuffer cmdBuffer, vk::ImageMemoryBarrier barrier);
}