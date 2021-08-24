#include "vkcv/Handles.hpp"

#include <iostream>

namespace vkcv {
	
	Handle::Handle() :
	m_id(UINT64_MAX), m_rc(nullptr), m_destroy(nullptr)
	{}
	
	Handle::Handle(uint64_t id, const HandleDestroyFunction& destroy) :
		m_id(id), m_rc(new uint64_t(1)), m_destroy(destroy)
	{}
	
	Handle::~Handle() {
		if ((m_rc) && (*m_rc > 0) && (--(*m_rc) == 0)) {
			if (m_destroy) {
				m_destroy(m_id);
			}
			
			delete m_rc;
		}
	}
	
	Handle::Handle(const Handle &other) :
		m_id(other.m_id),
		m_rc(other.m_rc),
		m_destroy(other.m_destroy)
	{
		if (m_rc) {
			++(*m_rc);
		}
	}
	
	Handle::Handle(Handle &&other) noexcept :
		m_id(other.m_id),
		m_rc(other.m_rc),
		m_destroy(other.m_destroy)
	{
		other.m_rc = nullptr;
	}
	
	Handle &Handle::operator=(const Handle &other) {
		if (&other == this) {
			return *this;
		}
		
		m_id = other.m_id;
		m_rc = other.m_rc;
		m_destroy = other.m_destroy;
		
		if (m_rc) {
			++(*m_rc);
		}
		
		return *this;
	}
	
	Handle &Handle::operator=(Handle &&other) noexcept {
		m_id = other.m_id;
		m_rc = other.m_rc;
		m_destroy = other.m_destroy;
		
		other.m_rc = nullptr;
		
		return *this;
	}
	
	uint64_t Handle::getId() const {
		return m_id;
	}
	
	uint64_t Handle::getRC() const {
		return m_rc? *m_rc : 0;
	}
	
	Handle::operator bool() const {
		return (m_id < UINT64_MAX);
	}
	
	bool Handle::operator!() const {
		return (m_id == UINT64_MAX);
	}
	
	std::ostream& operator << (std::ostream& out, const Handle& handle) {
		if (handle) {
			return out << "[" << typeid(handle).name() << ": " << handle.getId() << ":" << handle.getRC() << "]";
		} else {
			return out << "[" << typeid(handle).name() << ": none]";
		}
	}
	
	bool ImageHandle::isSwapchainImage() const {
		return (getId() == UINT64_MAX - 1);
	}
	
	ImageHandle ImageHandle::createSwapchainImageHandle(const HandleDestroyFunction &destroy) {
		return ImageHandle(uint64_t(UINT64_MAX - 1), destroy);
	}
	
}
