#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkcv
{
	enum class BufferType {
		VERTEX,
		UNIFORM,
		STORAGE,
		STAGING
	};
	
	enum class BufferMemoryType {
		DEVICE_LOCAL,
		HOST_VISIBLE
	};
	
	class Core;
	
	class BufferManager
	{
		friend class Core;
	private:
		
		struct Buffer
		{
			vk::Buffer m_handle;
			vk::DeviceMemory m_memory;
			size_t m_size;
			void* m_mapped = nullptr;
			bool m_mappable;
		};
		
		Core* m_core;
		std::vector<Buffer> m_buffers;
		uint64_t m_stagingBuffer;
		
		BufferManager() noexcept;
		
		void init();
		
	public:
		~BufferManager() noexcept;
		
		BufferManager(BufferManager&& other) = delete;
		BufferManager(const BufferManager& other) = delete;
		
		BufferManager& operator=(BufferManager&& other) = delete;
		BufferManager& operator=(const BufferManager& other) = delete;
		
		/**
		 * Creates and allocates a new buffer and returns its
		 * unique buffer handle id.
		 *
		 * @param type Type of buffer
		 * @param size Size of buffer in bytes
		 * @param memoryType Type of buffers memory
		 * @return New buffer handle id
		 */
		uint64_t createBuffer(BufferType type, size_t size, BufferMemoryType memoryType);
		
		/**
		 * Fills a buffer represented by a given buffer
		 * handle id with custom data.
		 *
		 * @param id Buffer handle id
		 * @param data Pointer to data
		 * @param size Size of data in bytes
		 * @param offset Offset to fill in data in bytes
		 */
		void fillBuffer(uint64_t id, void* data, size_t size, size_t offset);
		
		/**
		 * Maps memory to a buffer represented by a given
		 * buffer handle id and returns it.
		 *
		 * @param id Buffer handle id
		 * @param offset Offset of mapping in bytes
		 * @param size Size of mapping in bytes
		 * @return Pointer to mapped memory
		 */
		void* mapBuffer(uint64_t id, size_t offset, size_t size);
		
		/**
		 * Unmaps memory from a buffer represented by a given
		 * buffer handle id.
		 *
		 * @param id Buffer handle id
		 */
		void unmapBuffer(uint64_t id);
	
		/**
		 * Destroys and deallocates buffer represented by a given
		 * buffer handle id.
		 *
		 * @param id Buffer handle id
		 */
		void destroyBuffer(uint64_t id);
		
	};
	
}