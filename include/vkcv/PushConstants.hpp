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

    /**
     * @brief Class to handle push constants data per drawcall.
     */
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
		 * @brief Returns the size of the data that is bound
		 * per drawcall in bytes.
		 *
		 * @return Size of data per drawcall
		 */
		[[nodiscard]]
		size_t getSizePerDrawcall() const {
			return m_sizePerDrawcall;
		}
		
		/**
		 * @brief Returns the size of total data stored for
		 * push constants in bytes
		 *
		 * @return Total size of data
		 */
		[[nodiscard]]
		size_t getFullSize() const {
			return m_data.size();
		}
		
		/**
		 * @brief Returns the number of drawcalls that data
		 * is stored for.
		 *
		 * @return Number of drawcalls
		 */
		[[nodiscard]]
		size_t getDrawcallCount() const {
			return (m_data.size() / m_sizePerDrawcall);
		}
		
		/**
		 * @brief Clears the data for all drawcalls currently.
		 * stored.
		*/
		void clear() {
			m_data.clear();
		}
		
		/**
		 * @brief Appends data for a single drawcall to the
		 * storage with a given type.
		 * 
		 * @tparam T Type of data (must match the size per drawcall)
		 * @param[in] value Data to append
		 * @return True, if operation was successfull, otherwise false
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
		 * @brief Returns the data of the drawcall by a given index
		 * as reference.
		 * 
		 * @tparam T Type of data
		 * @param[in] index Index of the drawcall
		 * @return Drawcall data
		 */
		template<typename T = uint8_t>
		T& getDrawcall(size_t index) {
			const size_t offset = (index * m_sizePerDrawcall);
			return *reinterpret_cast<T*>(m_data.data() + offset);
		}
		
		/**
		 * @brief Returns the data of the drawcall by a given index
		 * as const reference.
		 *
		 * @tparam T Type of data
		 * @param[in] index Index of the drawcall
		 * @return Drawcall data
		 */
		template<typename T = uint8_t>
		const T& getDrawcall(size_t index) const {
			const size_t offset = (index * m_sizePerDrawcall);
			return *reinterpret_cast<const T*>(m_data.data() + offset);
		}
		
		/**
		 * @brief Returns the data of the drawcall by a given index
		 * as a pointer.
		 * 
		 * @param[in] index Index of the drawcall
		 * @return Drawcall data
		 */
		[[nodiscard]]
		const void* getDrawcallData(size_t index) const {
			const size_t offset = (index * m_sizePerDrawcall);
			return reinterpret_cast<const void*>(m_data.data() + offset);
		}
		
		/**
		 * @brief Returns the pointer to the entire drawcall data which
		 * might be nullptr if the data is empty.
		 *
		 * @return Pointer to the data
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
