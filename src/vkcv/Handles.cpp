#include "vkcv/Handles.hpp"

namespace vkcv {
	
	Handle::Handle() :
		m_id(UINT64_MAX)
	{}
	
	Handle::Handle(uint64_t id) :
		m_id(id)
	{}
	
	uint64_t Handle::getId() const {
		return m_id;
	}
	
	Handle::operator bool() const {
		return (m_id < UINT64_MAX);
	}
	
	bool Handle::operator!() const {
		return (m_id == UINT64_MAX);
	}
	
	std::ostream& operator << (std::ostream& out, const Handle& handle) {
		if (handle) {
			return out << "[Handle: " << handle.getId() << "]";
		} else {
			return out << "[Handle: none]";
		}
	}
	
}
