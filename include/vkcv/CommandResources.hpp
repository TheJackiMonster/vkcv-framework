#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv {
	struct CommandResources {
		vk::CommandPool commandPool;
		vk::CommandBuffer commandBuffer;
	};

	CommandResources createDefaultCommandResources(const vk::Device& device, const int graphicFamilyIndex);
	void destroyCommandResources(const vk::Device& device, const CommandResources& resources);
}