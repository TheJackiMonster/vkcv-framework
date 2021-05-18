#include "vkcv/CommandResources.hpp"

namespace vkcv {
	CommandResources createDefaultCommandResources(const vk::Device& device, const int graphicFamilyIndex) {
		CommandResources resources;
		vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		vk::CommandPoolCreateInfo poolCreateInfo(flags, graphicFamilyIndex);
		resources.commandPool = device.createCommandPool(poolCreateInfo, nullptr, {});

		const int commandPoolCount = 1;
		vk::CommandBufferAllocateInfo allocateInfo(resources.commandPool, vk::CommandBufferLevel::ePrimary, commandPoolCount);
		const std::vector<vk::CommandBuffer> createdBuffers = device.allocateCommandBuffers(allocateInfo, {});
		assert(createdBuffers.size() == 1);
		resources.commandBuffer = createdBuffers[0];

		return resources;
	}

	void destroyCommandResources(const vk::Device& device, const CommandResources& resources) {
		device.freeCommandBuffers(resources.commandPool, resources.commandBuffer, {});
		device.destroyCommandPool(resources.commandPool, {});
	}
}