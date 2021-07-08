#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkcv {
	
	class PushConstants {
	private:
		std::vector<uint8_t> m_data;
		size_t m_sizePerDrawcall;
		
	public:
		template<typename T>
		PushConstants() : PushConstants(sizeof(T)) {}
		
		explicit PushConstants(size_t sizePerDrawcall) :
		m_data(),
		m_sizePerDrawcall(sizePerDrawcall) {}
		
		PushConstants(const PushConstants& other) = default;
		PushConstants(PushConstants&& other) = default;
		
		~PushConstants() = default;
		
		PushConstants& operator=(const PushConstants& other) = default;
		PushConstants& operator=(PushConstants&& other) = default;
		
		[[nodiscard]]
		size_t getSizePerDrawcall() const {
			return m_sizePerDrawcall;
		}
		
		[[nodiscard]]
		size_t getFullSize() const {
			return m_data.size();
		}
		
		[[nodiscard]]
		size_t getDrawcallCount() const {
			return (m_data.size() / m_sizePerDrawcall);
		}
		
		void clear() {
			m_data.clear();
		}
		
		template<typename T = uint8_t>
		bool appendDrawcall(const T& value) {
			if (sizeof(T) != m_sizePerDrawcall) {
				return false;
			}
			
			const size_t offset = m_data.size();
			m_data.resize(offset + sizeof(value));
			std::memcpy(m_data.data() + offset, &value, sizeof(value));
			return true;
		}
		
		template<typename T = uint8_t>
		T& getDrawcall(size_t index) {
			const size_t offset = (index * m_sizePerDrawcall);
			return *reinterpret_cast<T*>(m_data.data() + offset);
		}
		
		template<typename T = uint8_t>
		const T& getDrawcall(size_t index) const {
			const size_t offset = (index * m_sizePerDrawcall);
			return *reinterpret_cast<const T*>(m_data.data() + offset);
		}
		
		[[nodiscard]]
		const void* getDrawcallData(size_t index) const {
			const size_t offset = (index * m_sizePerDrawcall);
			return reinterpret_cast<const void*>(m_data.data() + offset);
		}
		
		[[nodiscard]]
		const void* getData() const {
			if (m_data.empty()) {
				return nullptr;
			} else {
				return m_data.data();
			}
		}
		
	};
	
}
