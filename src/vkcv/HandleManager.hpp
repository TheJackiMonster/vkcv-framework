#pragma once

#include <optional>

#include "vkcv/Container.hpp"
#include "vkcv/Handles.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv {

	class Core;

	template <typename T, typename H = Handle>
	class HandleManager {
		friend class Core;

	private:
		Core* m_core;
		Vector<T> m_entries;

	protected:
		HandleManager() noexcept : m_core(nullptr), m_entries() {}

		virtual bool init(Core &core) {
			if (m_core) {
				vkcv_log(vkcv::LogLevel::ERROR, "Manager is already initialized");
				return false;
			}

			if (!m_entries.empty()) {
				vkcv_log(vkcv::LogLevel::WARNING,
						 "Entries added before initialization will be erased");
			}

			m_core = &core;
			m_entries.clear();
			return true;
		}

		[[nodiscard]] const Core &getCore() const {
			return *m_core;
		}

		[[nodiscard]] Core &getCore() {
			return *m_core;
		}

		[[nodiscard]] size_t getCount() const {
			return m_entries.size();
		}

		[[nodiscard]] const T &getById(uint64_t id) const {
			if (id >= m_entries.size()) {
				static T invalid;
				vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle id");
				return invalid;
			}

			return m_entries [id];
		}

		[[nodiscard]] T &getById(uint64_t id) {
			if (id >= m_entries.size()) {
				static T invalid;
				vkcv_log(vkcv::LogLevel::ERROR, "Invalid handle id");
				return invalid;
			}

			return m_entries [id];
		}

		virtual uint64_t getIdFrom(const H &handle) const = 0;

		[[nodiscard]] virtual const T &operator[](const H &handle) const {
			const Handle &_handle = handle;
			return getById(getIdFrom(static_cast<const H &>(_handle)));
		}

		[[nodiscard]] virtual T &operator[](const H &handle) {
			const Handle &_handle = handle;
			return getById(getIdFrom(static_cast<const H &>(_handle)));
		}

		H add(const T &entry) {
			const uint64_t id = m_entries.size();
			m_entries.push_back(entry);
			return createById(id, [&](uint64_t id) {
				destroyById(id);
			});
		}

		virtual H createById(uint64_t id, const HandleDestroyFunction &destroy) = 0;

		virtual void destroyById(uint64_t id) = 0;

		void clear() {
			for (uint64_t id = 0; id < m_entries.size(); id++) {
				destroyById(id);
			}
		}

	public:
		HandleManager(HandleManager &&other) = delete;
		HandleManager(const HandleManager &other) = delete;

		HandleManager &operator=(HandleManager &&other) = delete;
		HandleManager &operator=(const HandleManager &other) = delete;

		virtual ~HandleManager() noexcept = default;
	};

} // namespace vkcv
