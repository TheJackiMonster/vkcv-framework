#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkcv
{
	enum BufferType { VERTEX, UNIFORM, STORAGE };
	
	class BufferManager
	{
		friend class Core;
	private:
		
		struct Buffer
		{
			vk::Buffer m_handle;
			vk::DeviceMemory m_memory;
			void* m_mapped = nullptr;
		};
		
		vk::PhysicalDevice m_physicalDevice;
		vk::Device m_device;
		
		std::vector<Buffer> m_buffers;
		
		BufferManager(vk::Device device, vk::PhysicalDevice physicalDevice) noexcept;
		
	public:
		BufferManager() = delete;
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
		 * @return New buffer handle id
		 */
		uint64_t createBuffer(BufferType type, size_t size);
		
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
