#pragma once
/**
 * @authors Tobias Frisch, Alexander Gauggel, Artur Wasmut, Lars Hoerttrich, Sebastian Gaida
 * @file vkcv/BufferManager.hpp
 * @brief Manager to handle buffer operations.
 */

#include <memory>

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "vkcv/BufferTypes.hpp"
#include "vkcv/Container.hpp"
#include "vkcv/TypeGuard.hpp"

#include "HandleManager.hpp"

namespace vkcv {

	struct BufferEntry {
		TypeGuard m_typeGuard;

		BufferType m_type;
		BufferMemoryType m_memoryType;
		size_t m_size;

		vk::Buffer m_handle;
		vma::Allocation m_allocation;

		bool m_readable;
		bool m_mappable;
		char *m_mapping;
		size_t m_mapCounter;
	};

	/**
	 * @brief Class to manage the creation, destruction, allocation
	 * and filling of buffers.
	 */
	class BufferManager : public HandleManager<BufferEntry, BufferHandle> {
		friend class Core;

	private:
		std::allocator<char> m_allocator;
		
		bool m_resizableBar;
		bool m_shaderDeviceAddress;
		
		BufferHandle m_stagingBuffer;

		bool init(Core &core) override;

		[[nodiscard]] uint64_t getIdFrom(const BufferHandle &handle) const override;

		[[nodiscard]] BufferHandle createById(uint64_t id,
											  const HandleDestroyFunction &destroy) override;

		/**
		 * Destroys and deallocates buffer represented by a given
		 * buffer handle id.
		 *
		 * @param id Buffer handle id
		 */
		void destroyById(uint64_t id) override;

	public:
		BufferManager() noexcept;

		~BufferManager() noexcept override;

		/**
		 * @brief Creates and allocates a new buffer and returns its
		 * unique buffer handle.
		 *
		 * @param[in] typeGuard Type guard
		 * @param[in] type Type of buffer
		 * @param[in] memoryType Type of buffers memory
		 * @param[in] size Size of buffer in bytes
		 * @param[in] supportIndirect Support of indirect usage
		 * @param[in] readable Support read functionality
		 * @param[in] alignment (optional) Alignment for buffer memory
		 * @return New buffer handle
		 */
		[[nodiscard]] BufferHandle createBuffer(const TypeGuard &typeGuard, BufferType type,
												BufferMemoryType memoryType, size_t size,
												bool readable, size_t alignment = 0);

		/**
		 * @brief Returns the Vulkan buffer handle of a buffer
		 * represented by a given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Vulkan buffer handle
		 */
		[[nodiscard]] vk::Buffer getBuffer(const BufferHandle &handle) const;

		/**
		 * @brief Returns the type guard of a buffer represented
		 * by a given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Type guard
		 */
		[[nodiscard]] TypeGuard getTypeGuard(const BufferHandle &handle) const;

		/**
		 * @brief Returns the buffer type of a buffer represented
		 * by a given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Buffer type
		 */
		[[nodiscard]] BufferType getBufferType(const BufferHandle &handle) const;

		/**
		 * @brief Returns the buffer memory type of a buffer
		 * represented by a given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Buffer memory type
		 */
		[[nodiscard]] BufferMemoryType getBufferMemoryType(const BufferHandle &handle) const;

		/**
		 * @brief Returns the size of a buffer represented
		 * by a given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Size of the buffer
		 */
		[[nodiscard]] size_t getBufferSize(const BufferHandle &handle) const;

		/**
		 * @brief Returns the Vulkan device memory handle of a buffer
		 * represented by a given buffer handle id.
		 *
		 * @param[in] handle Buffer handle
		 * @return Vulkan device memory handle
		 */
		[[nodiscard]] vk::DeviceMemory getDeviceMemory(const BufferHandle &handle) const;
		
		/**
		 * @brief Returns the Vulkan device address of a buffer
		 * represented by a given buffer handle id.
		 *
		 * @param[in] handle Buffer handle
		 * @return Vulkan device address
		 */
		[[nodiscard]] vk::DeviceAddress getBufferDeviceAddress(const BufferHandle &handle) const;

		/**
		 * @brief Fills a buffer represented by a given buffer
		 * handle with custom data.
		 *
		 * @param[in] handle Buffer handle
		 * @param[in] data Pointer to data
		 * @param[in] size Size of data in bytes
		 * @param[in] offset Offset to fill in data in bytes
		 * @param[in] forceStaging (Optional) Flag to enforce transfer via staging buffer
		 */
		void fillBuffer(const BufferHandle &handle,
						const void* data,
						size_t size,
						size_t offset,
						bool forceStaging = false);

		/**
		 * @brief Reads from a buffer represented by a given
		 * buffer handle to some data pointer.
		 *
		 * @param[in] handle Buffer handle
		 * @param[in] data Pointer to data
		 * @param[in] size Size of data to read in bytes
		 * @param[in] offset Offset to read from buffer in bytes
		 */
		void readBuffer(const BufferHandle &handle, void* data, size_t size, size_t offset);

		/**
		 * @brief Maps memory to a buffer represented by a given
		 * buffer handle and returns it.
		 *
		 * @param[in] handle Buffer handle
		 * @param[in] offset Offset of mapping in bytes
		 * @param[in] size Size of mapping in bytes
		 * @return Pointer to mapped memory
		 */
		void* mapBuffer(const BufferHandle &handle, size_t offset, size_t size);

		/**
		 * @brief Unmaps memory from a buffer represented by a given
		 * buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 */
		void unmapBuffer(const BufferHandle &handle);

		/**
		 * @brief Records a memory barrier for a buffer,
		 * synchronizing subsequent accesses to buffer data
		 *
		 * @param[in] handle BufferHandle of the buffer
		 * @param[in] cmdBuffer Vulkan command buffer to record the barrier into
		 */
		void recordBufferMemoryBarrier(const BufferHandle &handle, vk::CommandBuffer cmdBuffer);
	};

} // namespace vkcv
