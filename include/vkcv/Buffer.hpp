#pragma once
/**
 * @authors Lars Hoerttrich, Tobias Frisch
 * @file vkcv/Buffer.hpp
 * @brief template buffer class, template for type security, implemented here because template classes can't be written in .cpp
 */
#include "Handles.hpp"
#include "BufferManager.hpp"

#include <vector>

namespace vkcv {

	template<typename T>
	class Buffer {
		friend class Core;
	public:
		// explicit destruction of default constructor
		Buffer<T>() = delete;
		
		[[nodiscard]]
		const BufferHandle& getHandle() const {
			return m_handle;
		}
		
		[[nodiscard]]
		BufferType getType() const {
			return m_type;
		};
		
		[[nodiscard]]
		size_t getCount() const {
			return m_count;
		}
		
		[[nodiscard]]
		size_t getSize() const {
			return m_count * sizeof(T);
		}

        [[nodiscard]]
        const vk::Buffer getVulkanHandle() const {
            return m_manager->getBuffer(m_handle);
        }
		
		void fill(const T* data, size_t count = 0, size_t offset = 0) {
			 m_manager->fillBuffer(m_handle, data, count * sizeof(T), offset * sizeof(T));
		}
		
		void fill(const std::vector<T>& vector, size_t offset = 0) {
			fill( static_cast<const T*>(vector.data()), static_cast<size_t>(vector.size()), offset);
		}
		
		[[nodiscard]]
		T* map(size_t offset = 0, size_t count = 0) {
			return reinterpret_cast<T*>(m_manager->mapBuffer(m_handle, offset * sizeof(T), count * sizeof(T)));
		}

		void unmap() {
			m_manager->unmapBuffer(m_handle);
		}

	private:
		BufferManager* const m_manager;
		const BufferHandle m_handle;
		const BufferType m_type;
		const size_t m_count;
		const BufferMemoryType m_memoryType;
		
		Buffer<T>(BufferManager* manager, BufferHandle handle, BufferType type, size_t count, BufferMemoryType memoryType) :
				m_manager(manager),
				m_handle(handle),
				m_type(type),
				m_count(count),
				m_memoryType(memoryType)
		{}
		
		[[nodiscard]]
		static Buffer<T> create(BufferManager* manager, BufferType type, size_t count, BufferMemoryType memoryType) {
			return Buffer<T>(manager, manager->createBuffer(type, count * sizeof(T), memoryType), type, count, memoryType);
		}
		
	};
}
