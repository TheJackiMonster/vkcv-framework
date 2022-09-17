#include "CommandStreamManager.hpp"
#include "vkcv/Core.hpp"

#include "vkcv/Logger.hpp"

namespace vkcv {

	uint64_t CommandStreamManager::getIdFrom(const CommandStreamHandle &handle) const {
		return handle.getId();
	}

	CommandStreamHandle CommandStreamManager::createById(uint64_t id,
														 const HandleDestroyFunction &destroy) {
		return CommandStreamHandle(id, destroy);
	}

	void CommandStreamManager::destroyById(uint64_t id) {
		auto &stream = getById(id);

		if (stream.cmdBuffer) {
			getCore().getContext().getDevice().freeCommandBuffers(stream.cmdPool, stream.cmdBuffer);
			stream.cmdBuffer = nullptr;
			stream.callbacks.clear();
		}
	}

	CommandStreamManager::CommandStreamManager() noexcept :
		HandleManager<CommandStreamEntry, CommandStreamHandle>() {}

	CommandStreamManager::~CommandStreamManager() noexcept {
		clear();
	}

	CommandStreamHandle CommandStreamManager::createCommandStream(const vk::Queue &queue,
																  vk::CommandPool cmdPool) {
		const vk::CommandBufferAllocateInfo info(cmdPool, vk::CommandBufferLevel::ePrimary, 1);
		auto &device = getCore().getContext().getDevice();

		const vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers(info).front();

		const vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdBuffer.begin(beginInfo);

		for (uint64_t id = 0; id < getCount(); id++) {
			auto &stream = getById(id);

			if (!(stream.cmdBuffer)) {
				stream.cmdBuffer = cmdBuffer;
				stream.cmdPool = cmdPool;
				stream.queue = queue;

				return createById(id, [&](uint64_t id) {
					destroyById(id);
				});
			}
		}

		return add({ cmdBuffer, cmdPool, queue, {} });
	}

	void CommandStreamManager::recordCommandsToStream(const CommandStreamHandle &handle,
													  const RecordCommandFunction &record) {
		auto &stream = (*this) [handle];
		record(stream.cmdBuffer);
	}

	void CommandStreamManager::addFinishCallbackToStream(const CommandStreamHandle &handle,
														 const FinishCommandFunction &finish) {
		auto &stream = (*this) [handle];
		stream.callbacks.push_back(finish);
	}

	void CommandStreamManager::submitCommandStreamSynchronous(
		const CommandStreamHandle &handle, std::vector<vk::Semaphore> &waitSemaphores,
		std::vector<vk::Semaphore> &signalSemaphores) {
		auto &stream = (*this) [handle];
		stream.cmdBuffer.end();

		const auto device = getCore().getContext().getDevice();
		const vk::Fence waitFence = device.createFence({});

		const std::vector<vk::PipelineStageFlags> waitDstStageMasks(
			waitSemaphores.size(), vk::PipelineStageFlagBits::eAllCommands);

		const vk::SubmitInfo queueSubmitInfo(waitSemaphores, waitDstStageMasks, stream.cmdBuffer,
											 signalSemaphores);

		stream.queue.submit(queueSubmitInfo, waitFence);
		assert(device.waitForFences(waitFence, true, UINT64_MAX) == vk::Result::eSuccess);

		device.destroyFence(waitFence);
		stream.queue = nullptr;

		for (const auto &finishCallback : stream.callbacks) {
			finishCallback();
		}
	}

	vk::CommandBuffer
	CommandStreamManager::getStreamCommandBuffer(const CommandStreamHandle &handle) {
		auto &stream = (*this) [handle];
		return stream.cmdBuffer;
	}

} // namespace vkcv