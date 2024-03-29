#pragma once
/**
 * @authors Tobias Frisch, Alexander Gauggel
 * @file vkcv/PushConstants.hpp
 * @brief Class to manage push constants for pipeline recording.
 */

#include <vulkan/vulkan.hpp>

#include "Container.hpp"
#include "Logger.hpp"
#include "TypeGuard.hpp"

namespace vkcv {

	/**
	 * @brief Class to handle push constants data per drawcall.
	 */
	class PushConstants final {
	private:
		TypeGuard m_typeGuard;
		Vector<uint8_t> m_data;

	public:
		explicit PushConstants(size_t sizePerDrawcall);
		explicit PushConstants(const TypeGuard &guard);

		PushConstants(const PushConstants &other) = default;
		PushConstants(PushConstants &&other) = default;

		~PushConstants() = default;

		PushConstants &operator=(const PushConstants &other) = default;
		PushConstants &operator=(PushConstants &&other) = default;

		/**
		 * @brief Returns the size of the data that is bound
		 * per drawcall in bytes.
		 *
		 * @return Size of data per drawcall
		 */
		[[nodiscard]] size_t getSizePerDrawcall() const;

		/**
		 * @brief Returns the size of total data stored for
		 * push constants in bytes
		 *
		 * @return Total size of data
		 */
		[[nodiscard]] size_t getFullSize() const;

		/**
		 * @brief Returns the number of drawcalls that data
		 * is stored for.
		 *
		 * @return Number of drawcalls
		 */
		[[nodiscard]] size_t getDrawcallCount() const;

		/**
		 * @brief Clears the data for all drawcalls currently.
		 * stored.
		 */
		void clear();

		/**
		 * @brief Appends data for a single drawcall to the
		 * storage with a given type.
		 *
		 * @tparam T Type of data (must match the size per drawcall)
		 * @param[in] value Data to append
		 * @return True, if operation was successfull, otherwise false
		 */
		template <typename T = uint8_t>
		bool appendDrawcall(const T &value) {
			if (!m_typeGuard.template check<T>()) {
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
		template <typename T = uint8_t>
		T &getDrawcall(size_t index) {
			const size_t offset = (index * getSizePerDrawcall());
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
		template <typename T = uint8_t>
		const T &getDrawcall(size_t index) const {
			const size_t offset = (index * getSizePerDrawcall());
			return *reinterpret_cast<const T*>(m_data.data() + offset);
		}

		/**
		 * @brief Returns the data of the drawcall by a given index
		 * as a pointer.
		 *
		 * @param[in] index Index of the drawcall
		 * @return Drawcall data
		 */
		[[nodiscard]] const void* getDrawcallData(size_t index) const;

		/**
		 * @brief Returns the pointer to the entire drawcall data which
		 * might be nullptr if the data is empty.
		 *
		 * @return Pointer to the data
		 */
		[[nodiscard]] const void* getData() const;
	};

	template <typename T>
	PushConstants pushConstants() {
		return PushConstants(typeGuard<T>());
	}

	template <typename T>
	PushConstants pushConstants(const T &value) {
		auto pc = pushConstants<T>();
		pc.template appendDrawcall<T>(value);
		return pc;
	}

	template <typename T>
	PushConstants pushConstants(const Vector<T> &values) {
		auto pc = pushConstants<T>();

		for (const T &value : values) {
			if (!(pc.template appendDrawcall<T>(value))) {
				break;
			}
		}

		return pc;
	}

} // namespace vkcv
