#pragma once
/**
 * @authors Tobias Frisch, Alexander Gauggel
 * @file vkcv/PushConstants.hpp
 * @brief Class to manage push constants for pipeline recording.
 */

#include <vector>
#include <vulkan/vulkan.hpp>

#include "Logger.hpp"

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
		
		/**
		 * @return The size of the data that is bound per drawcall in bytes
		 */
		[[nodiscard]]
		size_t getSizePerDrawcall() const {
			return m_sizePerDrawcall;
		}
		
		/**
		 * @return The size of the total data stored for push constants in bytes
		 */
		[[nodiscard]]
		size_t getFullSize() const {
			return m_data.size();
		}
		
		/**
		 * @return The number of drawcalls that data is stored for
		 */
		[[nodiscard]]
		size_t getDrawcallCount() const {
			return (m_data.size() / m_sizePerDrawcall);
		}
		
		/**
		 * @brief Clear the drawcall data
		*/
		void clear() {
			m_data.clear();
		}
		
		/**
		 * @brief Append data for a single drawcall
		 * 
		 * @tparam T Type of data to append, must match the size of the #PushConstants per drawcall size
		 * @param value Data to append
		 * @return If operation was successfull
		 */
		template<typename T = uint8_t>
		bool appendDrawcall(const T& value) {
			if (sizeof(T) != m_sizePerDrawcall) {
				vkcv_log(LogLevel::WARNING, "Size (%lu) of value does not match the specified size per drawcall (%lu)",
						 sizeof(value), m_sizePerDrawcall);
				return false;
			}
			
			const size_t offset = m_data.size();
			m_data.resize(offset + sizeof(value));
			std::memcpy(m_data.data() + offset, &value, sizeof(value));
			return true;
		}
		
		/**
		 * @brief Get the data for a single drawcall as reference
		 * 
		 * @tparam T Type of data to return
		 * @param index Index of the drawcall data to return
		 * @return Drawcall data
		 */
		template<typename T = uint8_t>
		T& getDrawcall(size_t index) {
			const size_t offset = (index * m_sizePerDrawcall);
			return *reinterpret_cast<T*>(m_data.data() + offset);
		}
		
		/**
		 * @brief Get the data for a single drawcall as const reference
		 *
		 * @tparam T Type of data to return
		 * @param index Index of the drawcall data to return
		 * @return Drawcall data
		 */
		template<typename T = uint8_t>
		const T& getDrawcall(size_t index) const {
			const size_t offset = (index * m_sizePerDrawcall);
			return *reinterpret_cast<const T*>(m_data.data() + offset);
		}
		
		/**
		 * @brief Get the data for a single drawcall as a void pointer
		 * 
		 * @param index Index of the drawcall data to return
		 * @return Drawcall data
		 */
		[[nodiscard]]
		const void* getDrawcallData(size_t index) const {
			const size_t offset = (index * m_sizePerDrawcall);
			return reinterpret_cast<const void*>(m_data.data() + offset);
		}
		
		/**
		 * @return Raw pointer to the entire drawcall data array, 
		 * might be nullptr if data is empty, 
		 * pointer might be invalidated by clearing or adding data
		 */
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
