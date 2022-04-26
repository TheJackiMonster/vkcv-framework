#pragma once
/**
 * @authors Tobias Frisch, Alexander Gauggel, Artur Wasmut, Lars Hoerttrich, Sebastian Gaida
 * @file vkcv/BufferManager.hpp
 * @brief Manager to handle buffer operations.
 */

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
		STAGING,
		INDIRECT
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
		 * @brief Creates and allocates a new buffer and returns its
		 * unique buffer handle.
		 *
		 * @param[in] type Type of buffer
		 * @param[in] size Size of buffer in bytes
		 * @param[in] memoryType Type of buffers memory
		 * @param[in] supportIndirect Support of indirect usage
		 * @return New buffer handle
		 */
		BufferHandle createBuffer(BufferType type,
								  size_t size,
								  BufferMemoryType memoryType,
								  bool supportIndirect);
		
		/**
		 * @brief Returns the Vulkan buffer handle of a buffer
		 * represented by a given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Vulkan buffer handle
		 */
		[[nodiscard]]
		vk::Buffer getBuffer(const BufferHandle& handle) const;
		
		/**
		 * @brief Returns the size of a buffer represented
		 * by a given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Size of the buffer
		 */
		[[nodiscard]]
		size_t getBufferSize(const BufferHandle& handle) const;
		
		/**
		 * @brief Returns the Vulkan device memory handle of a buffer
		 * represented by a given buffer handle id.
		 *
		 * @param[in] handle Buffer handle
		 * @return Vulkan device memory handle
		 */
		[[nodiscard]]
		vk::DeviceMemory getDeviceMemory(const BufferHandle& handle) const;
		
		/**
		 * @brief Fills a buffer represented by a given buffer
		 * handle with custom data.
		 *
		 * @param[in] handle Buffer handle
		 * @param[in] data Pointer to data
		 * @param[in] size Size of data in bytes
		 * @param[in] offset Offset to fill in data in bytes
		 */
		void fillBuffer(const BufferHandle& handle,
						const void* data,
						size_t size,
						size_t offset);
		
		/**
		 * @brief Maps memory to a buffer represented by a given
		 * buffer handle and returns it.
		 *
		 * @param[in] handle Buffer handle
		 * @param[in] offset Offset of mapping in bytes
		 * @param[in] size Size of mapping in bytes
		 * @return Pointer to mapped memory
		 */
		void* mapBuffer(const BufferHandle& handle,
						size_t offset,
						size_t size);
		
		/**
		 * @brief Unmaps memory from a buffer represented by a given
		 * buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 */
		void unmapBuffer(const BufferHandle& handle);
		
		/**
		 * @brief Records a memory barrier for a buffer,
		 * synchronizing subsequent accesses to buffer data
		 *
		 * @param[in] handle BufferHandle of the buffer
		 * @param[in] cmdBuffer Vulkan command buffer to record the barrier into
		*/
		void recordBufferMemoryBarrier(const BufferHandle& handle,
									   vk::CommandBuffer cmdBuffer);
	};
	
}
