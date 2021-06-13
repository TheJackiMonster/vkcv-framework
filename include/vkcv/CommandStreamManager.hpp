#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include "vkcv/Event.hpp"
#include "vkcv/Handles.hpp"
#include "vkcv/CommandRecordingFunctionTypes.hpp"

namespace vkcv {
	
	class Core;

	class CommandStreamManager
	{
		friend class Core;
	private:
		struct CommandStream {
			inline CommandStream(vk::CommandBuffer cmdBuffer, vk::Queue queue, vk::CommandPool cmdPool) 
				: cmdBuffer(cmdBuffer), cmdPool(cmdPool), queue(queue) {};
			vk::CommandBuffer                   cmdBuffer;
			vk::CommandPool                     cmdPool;
			vk::Queue                           queue;
			std::vector<FinishCommandFunction>  callbacks;
		};

		Core* m_core;
		std::vector<CommandStream> m_commandStreams;

		CommandStreamManager() noexcept;

		void init(Core* core);

	public:
		~CommandStreamManager() noexcept;

		CommandStreamManager(CommandStreamManager&& other) = delete;
		CommandStreamManager(const CommandStreamManager& other) = delete;

		CommandStreamManager& operator=(CommandStreamManager&& other) = delete;
		CommandStreamManager& operator=(const CommandStreamManager& other) = delete;

		CommandStreamHandle createCommandStream(
			const vk::Queue queue,
			vk::CommandPool cmdPool);

		void recordCommandsToStream(const CommandStreamHandle handle, const RecordCommandFunction record);
		void addFinishCallbackToStream(const CommandStreamHandle handle, const FinishCommandFunction finish);
		void submitCommandStreamSynchronous(
			const CommandStreamHandle   handle,
			std::vector<vk::Semaphore>  &waitSemaphores,
			std::vector<vk::Semaphore>  &signalSemaphores);

		vk::CommandBuffer getStreamCommandBuffer(const CommandStreamHandle handle);
	};

}