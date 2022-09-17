#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "vkcv/Event.hpp"
#include "vkcv/EventFunctionTypes.hpp"

#include "HandleManager.hpp"

namespace vkcv {

	/**
	 * @brief Represents one command stream, into which commands can be recorded into.
	 * Consists of a command buffer, the command buffer's command pool and a queue, as well as some
	 * callbacks.
	 */
	struct CommandStreamEntry {
		vk::CommandBuffer cmdBuffer;
		vk::CommandPool cmdPool;
		vk::Queue queue;
		std::vector<FinishCommandFunction> callbacks;
	};

	/**
	 * @brief Responsible for creation, deletion, callbacks and recording of command streams
	 */
	class CommandStreamManager : public HandleManager<CommandStreamEntry, CommandStreamHandle> {
		friend class Core;

	private:
		[[nodiscard]] uint64_t getIdFrom(const CommandStreamHandle &handle) const override;

		[[nodiscard]] CommandStreamHandle createById(uint64_t id,
													 const HandleDestroyFunction &destroy) override;

		void destroyById(uint64_t id) override;

	public:
		CommandStreamManager() noexcept;

		~CommandStreamManager() noexcept override;

		/**
		 * @brief Creates a new command stream
		 *
		 * @param queue Queue the command buffer will be submitted to
		 * @param cmdPool Command pool the command buffer will be allocated from
		 * @return Handle that represents the #CommandStream
		 */
		CommandStreamHandle createCommandStream(const vk::Queue &queue, vk::CommandPool cmdPool);

		/**
		 * @brief Record vulkan commands to a #CommandStream, using a record function
		 *
		 * @param handle Command stream handle
		 * @param record Function that records the vulkan commands
		 */
		void recordCommandsToStream(const CommandStreamHandle &handle,
									const RecordCommandFunction &record);

		/**
		 * @brief Add a callback to a #CommandStream that is called
		 * every time the command stream is submitted and finished
		 *
		 * @param handle Command stream handle
		 * @param finish Callback that is called when a command stream submission is finished
		 */
		void addFinishCallbackToStream(const CommandStreamHandle &handle,
									   const FinishCommandFunction &finish);

		/**
		 * @brief Submits a #CommandStream to it's queue and returns after execution is finished
		 *
		 * @param handle Command stream handle
		 * @param waitSemaphores Semaphores that are waited upon before executing the recorded
		 * commands
		 * @param signalSemaphores Semaphores that are signaled when execution of the recorded
		 * commands is finished
		 */
		void submitCommandStreamSynchronous(const CommandStreamHandle &handle,
											std::vector<vk::Semaphore> &waitSemaphores,
											std::vector<vk::Semaphore> &signalSemaphores);

		/**
		 * @brief Returns the underlying vulkan handle of a #CommandStream to be used for manual
		 * command recording
		 *
		 * @param handle Command stream handle
		 * @return Vulkan handle of the #CommandStream
		 */
		vk::CommandBuffer getStreamCommandBuffer(const CommandStreamHandle &handle);
	};

} // namespace vkcv