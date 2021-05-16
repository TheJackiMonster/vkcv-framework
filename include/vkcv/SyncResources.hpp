#pragma once
#include <vulkan/vulkan.hpp>

namespace vkcv {
	struct SyncResources {
		vk::Semaphore renderFinished;
		vk::Fence presentFinished;
	};

	SyncResources createDefaultSyncResources(const vk::Device& device);
	void destroySyncResources(const vk::Device& device, const SyncResources& resources);
}