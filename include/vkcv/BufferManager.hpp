#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include "Handles.hpp"

namespace vkcv
{
	enum class BufferType {
		INDEX,
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
			vma::Allocation m_allocation;
			size_t m_size = 0;
			bool m_mappable = false;
		};
		
		Core* m_core;
		std::vector<Buffer> m_buffers;
		BufferHandle m_stagingBuffer;
		
		BufferManager() noexcept;
		
		void init();
		
		/**
		 * Destroys and deallocates buffer represented by a given
		 * buffer handle id.
		 *
		 * @param id Buffer handle id
		 */
		void destroyBufferById(uint64_t id);
		
	public:
		~BufferManager() noexcept;
		
		BufferManager(BufferManager&& other) = delete;
		BufferManager(const BufferManager& other) = delete;
		
		BufferManager& operator=(BufferManager&& other) = delete;
		BufferManager& operator=(const BufferManager& other) = delete;
		
		/**
		 * Creates and allocates a new buffer and returns its
		 * unique buffer handle.
		 *
		 * @param type Type of buffer
		 * @param size Size of buffer in bytes
		 * @param memoryType Type of buffers memory
		 * @return New buffer handle
		 */
		BufferHandle createBuffer(BufferType type, size_t size, BufferMemoryType memoryType);
		
		/**
		 * Returns the Vulkan buffer handle of a buffer
		 * represented by a given buffer handle.
		 *
		 * @param handle Buffer handle
		 * @return Vulkan buffer handle
		 */
		[[nodiscard]]
		vk::Buffer getBuffer(const BufferHandle& handle) const;
		
		/**
		 * Returns the size of a buffer represented
		 * by a given buffer handle.
		 *
		 * @param handle Buffer handle
		 * @return Size of the buffer
		 */
		[[nodiscard]]
		size_t getBufferSize(const BufferHandle& handle) const;
		
		/**
		 * Returns the Vulkan device memory handle of a buffer
		 * represented by a given buffer handle id.
		 *
		 * @param handle Buffer handle
		 * @return Vulkan device memory handle
		 */
		[[nodiscard]]
		vk::DeviceMemory getDeviceMemory(const BufferHandle& handle) const;
		
		/**
		 * Fills a buffer represented by a given buffer
		 * handle with custom data.
		 *
		 * @param handle Buffer handle
		 * @param data Pointer to data
		 * @param size Size of data in bytes
		 * @param offset Offset to fill in data in bytes
		 */
		void fillBuffer(const BufferHandle& handle, const void* data, size_t size, size_t offset);
		
		/**
		 * Maps memory to a buffer represented by a given
		 * buffer handle and returns it.
		 *
		 * @param handle Buffer handle
		 * @param offset Offset of mapping in bytes
		 * @param size Size of mapping in bytes
		 * @return Pointer to mapped memory
		 */
		void* mapBuffer(const BufferHandle& handle, size_t offset, size_t size);
		
		/**
		 * Unmaps memory from a buffer represented by a given
		 * buffer handle.
		 *
		 * @param handle Buffer handle
		 */
		void unmapBuffer(const BufferHandle& handle);
		
		void recordBufferMemoryBarrier(
			const BufferHandle& handle,
			vk::CommandBuffer cmdBuffer);


		/**
		 * Reads buffer data from GPU for a given buffer handle
		 * and writes it to data pointer. Uses cmdBuffer to
		 * create neccessary memory barriers.
		 * 
		 * @param handle Buffer handle to read
		 * @param cmdBuffer commandbuffer to execute memory barriers
		 * @param data data pointer to which result will be written
		 *
		 */
		void readBuffer(
		    const BufferHandle& handle,
		    vk::CommandBuffer cmdBuffer,
		    void* data);
	};
	
}
