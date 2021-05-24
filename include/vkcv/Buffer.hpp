#pragma once
/**
 * @authors Lars Hoerttrich, Tobias Frisch
 * @file vkcv/Buffer.hpp
 * @brief template buffer class, template for type security, implemented here because template classes can't be written in .cpp
 */
#include "BufferManager.hpp"

namespace vkcv {

	template<typename T>
	class Buffer {
	public:
		// explicit destruction of default constructor
		Buffer<T>() = delete;

		BufferType getType() {
			return m_type;
		};
		
		size_t getCount() {
			return m_count;
		}
		
		size_t getSize() {
			return m_count * sizeof(T);
		}
		
		void fill(T* data, size_t count = SIZE_MAX, size_t offset = 0) {
			 m_manager->fillBuffer(m_handle_id, data, count * sizeof(T), offset * sizeof(T));
		}
		
		T* map(size_t offset = 0, size_t count = SIZE_MAX) {
			return reinterpret_cast<T*>(m_manager->mapBuffer(m_handle_id, offset * sizeof(T), count * sizeof(T)));
		}

		void unmap() {
			m_manager->unmapBuffer(m_handle_id);
		}
		
		static Buffer<T> create(BufferManager* manager, BufferType type, size_t count) {
			return Buffer<T>(manager, manager->createBuffer(type, count * sizeof(T)), type, count);
		}

	private:
		BufferManager* m_manager;
		uint64_t m_handle_id;
		BufferType m_type;
		size_t m_count;
		
		Buffer<T>(BufferManager* manager, uint64_t id, BufferType type, size_t count) :
				m_manager(manager),
				m_handle_id(id),
				m_type(type),
				m_count(count)
		{}
		
	};
}
