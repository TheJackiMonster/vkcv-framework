#pragma once
/**
 * @authors Tobias Frisch, Lars Hoerttrich, Alexander Gauggel
 * @file vkcv/Buffer.hpp
 * @brief Template buffer class for type security with buffers.
 */

#include <vector>

#include "BufferTypes.hpp"
#include "Core.hpp"
#include "Handles.hpp"

namespace vkcv {

	/**
	 * @brief Template class for buffer handling and filling data.
	 *
	 * @tparam T Buffer content type
	 */
	template<typename T>
	class Buffer {
	public:
		Buffer() : m_core(nullptr), m_handle() {}
		
		Buffer(Core* core, const BufferHandle& handle) : m_core(core), m_handle(handle) {}
		
		Buffer(const Buffer& other) = default;
		Buffer(Buffer&& other) = default;
		
		~Buffer() = default;
		
		Buffer& operator=(const Buffer& other) = default;
		Buffer& operator=(Buffer&& other) = default;
		
		/**
		 * @brief Returns the buffers handle.
		 *
		 * @return The #BufferHandle to be used with the #Core
		 */
		[[nodiscard]]
		const BufferHandle& getHandle() const {
			return m_handle;
		}
		
		/**
		 * @brief Returns the type of the buffer.
		 *
		 * @return The #BufferType of the #Buffer
		 */
		[[nodiscard]]
		BufferType getType() const {
			return m_core->getBufferType(m_handle);
		};
		
		[[nodiscard]]
		BufferMemoryType getMemoryType() const {
			return m_core->getBufferMemoryType(m_handle);
		}
		
		/**
		 * @brief Returns the count of elements in the buffer.
		 *
		 * @return The number of objects of type T the #Buffer holds
		 */
		[[nodiscard]]
		size_t getCount() const {
			return m_core->getBufferSize(m_handle) / sizeof(T);
		}
		
		/**
		 * @brief Returns the size of the buffer in bytes.
		 *
		 * @return The size of the #Buffer in bytes
		 */
		[[nodiscard]]
		size_t getSize() const {
			return m_core->getBufferSize(m_handle);
		}

		/**
		 * @brief Returns the vulkan buffer handle of the buffer.
		 *
		 * @return The vulkan handle of the #Buffer to be used for manual vulkan commands
		 */
        [[nodiscard]]
		vk::Buffer getVulkanHandle() const {
            return m_core->getBuffer(m_handle);
        }

		/**
		 * @brief Fills the #Buffer with data of type T.
		 * 
		 * @param[in] data Pointer to the array of object type T
		 * @param[in] count The number of objects to copy from the data array
		 * @param[in] offset The offset into the #Buffer where the data is copied into
		 */
		void fill(const T* data,
				  size_t count = 0,
				  size_t offset = 0) {
			 m_core->fillBuffer(m_handle, data, count * sizeof(T), offset * sizeof(T));
		}
		
		/**
		 * @brief Fills the #Buffer with data from a vector of type T.
		 * 
		 * @param vector Vector of type T to be copied into the #Buffer
		 * @param offset The offset into the #Buffer where the data is copied into
		 */
		void fill(const std::vector<T>& vector,
				  size_t offset = 0) {
			fill(static_cast<const T*>(vector.data()), static_cast<size_t>(vector.size()), offset);
		}
		
		/**
		 * @brief Reads the #Buffer directly into a data pointer of type T.
		 *
		 * @param[in] data Pointer to the array of object type T
		 * @param[in] count The number of objects to copy from the buffer
		 * @param[in] offset The offset into the #Buffer where the data is copied from
		 */
		void read(T* data,
				  size_t count = 0,
				  size_t offset = 0) {
			m_core->readBuffer(m_handle, data, count * sizeof(T), offset * sizeof(T));
		}
		
		/**
		 * @brief Reads the #Buffer directly to a vector of type T.
		 *
		 * @param vector Vector of type T to be copied into from the #Buffer
		 * @param offset The offset into the #Buffer where the data is copied from
		 */
		void read(std::vector<T>& vector,
				  size_t offset = 0) {
			read(static_cast<T*>(vector.data()), static_cast<size_t>(vector.size()), offset);
		}
		
		/**
		 * @brief Maps memory to the #Buffer and returns it.
		 *
		 * @param[in] offset Offset of mapping in objects of type T
		 * @param[in] count Count of objects of type T that are mapped
		 * @return Pointer to mapped memory as type T
		 */
		[[nodiscard]]
		T* map(size_t offset = 0,
			   size_t count = 0) {
			return reinterpret_cast<T*>(m_core->mapBuffer(m_handle, offset * sizeof(T), count * sizeof(T)));
		}

		/**
		 * @brief Unmaps the #Buffer, invalidates the pointer obtained by map().
		 */
		void unmap() {
			m_core->unmapBuffer(m_handle);
		}

	private:
		Core* m_core;
		BufferHandle m_handle;
		
	};
	
	template<typename T>
	Buffer<T> buffer(Core& core,
					 BufferType type,
					 size_t count,
					 BufferMemoryType memoryType = BufferMemoryType::DEVICE_LOCAL,
					 bool readable = false) {
		return Buffer<T>(&core, core.createBuffer(type, typeGuard<T>(), count, memoryType, readable));
	}
	
}
