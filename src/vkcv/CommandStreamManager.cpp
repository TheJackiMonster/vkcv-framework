#include "CommandStreamManager.hpp"
#include "vkcv/Core.hpp"

#include "vkcv/Logger.hpp"

namespace vkcv {
	CommandStreamManager::CommandStreamManager() noexcept : m_core(nullptr){}

	CommandStreamManager::~CommandStreamManager() noexcept {
		for (const auto& stream : m_commandStreams) {
			if (stream.cmdBuffer && stream.cmdBuffer) {
				m_core->getContext().getDevice().freeCommandBuffers(stream.cmdPool, stream.cmdBuffer);
			}
		}
	}

	void CommandStreamManager::init(Core* core) {
		if (!core) {
			vkcv_log(LogLevel::ERROR, "Requires valid core pointer");
		}
		m_core = core;
	}

	CommandStreamHandle CommandStreamManager::createCommandStream(
		const vk::Queue &queue,
		vk::CommandPool cmdPool) {

		const vk::CommandBuffer cmdBuffer = allocateCommandBuffer(m_core->getContext().getDevice(), cmdPool);

		CommandStream stream(cmdBuffer, queue, cmdPool);
		beginCommandBuffer(stream.cmdBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		// find unused stream
		int unusedStreamIndex = -1;
		for (size_t i = 0; i < m_commandStreams.size(); i++) {
			if (m_commandStreams[i].cmdBuffer) {
				// still in use
			} else {
				unusedStreamIndex = i;
				break;
			}
		}

        const bool foundUnusedStream = unusedStreamIndex >= 0;
        if (foundUnusedStream) {
            m_commandStreams[unusedStreamIndex] = stream;
            return CommandStreamHandle(unusedStreamIndex);
        }

        CommandStreamHandle handle(m_commandStreams.size());
        m_commandStreams.push_back(stream);
        return handle;
    }

	void CommandStreamManager::recordCommandsToStream(
		const CommandStreamHandle   &handle,
		const RecordCommandFunction &record) {

		const size_t id = handle.getId();
		if (id >= m_commandStreams.size()) {
			vkcv_log(LogLevel::ERROR, "Requires valid handle");
			return;
		}

		CommandStream& stream = m_commandStreams[id];
		record(stream.cmdBuffer);
	}

	void CommandStreamManager::addFinishCallbackToStream(
		const CommandStreamHandle   &handle,
		const FinishCommandFunction &finish) {

		const size_t id = handle.getId();
		if (id >= m_commandStreams.size()) {
			vkcv_log(LogLevel::ERROR, "Requires valid handle");
			return;
		}

		CommandStream& stream = m_commandStreams[id];
		stream.callbacks.push_back(finish);
	}

	void CommandStreamManager::submitCommandStreamSynchronous(
		const CommandStreamHandle   &handle,
		std::vector<vk::Semaphore>  &waitSemaphores,
		std::vector<vk::Semaphore>  &signalSemaphores) {

		const size_t id = handle.getId();
		if (id >= m_commandStreams.size()) {
			vkcv_log(LogLevel::ERROR, "Requires valid handle");
			return;
		}
		CommandStream& stream = m_commandStreams[id];
		stream.cmdBuffer.end();

		const auto device = m_core->getContext().getDevice();
		const vk::Fence waitFence = createFence(device);
		submitCommandBufferToQueue(stream.queue, stream.cmdBuffer, waitFence, waitSemaphores, signalSemaphores);
		waitForFence(device, waitFence);
		device.destroyFence(waitFence);

		device.freeCommandBuffers(stream.cmdPool, stream.cmdBuffer);
		stream.cmdBuffer    = nullptr;
		stream.cmdPool      = nullptr;
		stream.queue        = nullptr;

		for (const auto& finishCallback : stream.callbacks) {
			finishCallback();
		}
	}

	vk::CommandBuffer CommandStreamManager::getStreamCommandBuffer(const CommandStreamHandle &handle) {
		const size_t id = handle.getId();
		if (id >= m_commandStreams.size()) {
			vkcv_log(LogLevel::ERROR, "Requires valid handle");
			return nullptr;
		}
		return m_commandStreams[id].cmdBuffer;
	}
}