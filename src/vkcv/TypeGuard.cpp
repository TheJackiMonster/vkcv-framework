#include <vkcv/TypeGuard.hpp>

#include <vkcv/Logger.hpp>
#include <string.h>

namespace vkcv {
	
#ifndef NDEBUG
	bool TypeGuard::checkType(const char* name, size_t hash, size_t size) const {
		if (!checkTypeSize(size)) {
			return false;
		}
		
		if ((!m_typeName) || (!name)) {
			return true;
		}
		
		if (m_typeHash != hash) {
			vkcv_log(
					LogLevel::WARNING,
					"Hash (%lu) does not match the specified hash of the type guard (%lu)",
					hash,
					m_typeHash
			);
			
			return false;
		}
		
		if (strcmp(m_typeName, name) != 0) {
			vkcv_log(
					LogLevel::WARNING,
					"Name (%s) does not match the specified name of the type guard (%s)",
					name,
					m_typeName
			);
			
			return false;
		} else {
			return true;
		}
	}
#endif
	
	bool TypeGuard::checkTypeSize(size_t size) const {
		if (m_typeSize != size) {
			vkcv_log(
					LogLevel::WARNING,
					"Size (%lu) does not match the specified size of the type guard (%lu)",
					size,
					m_typeSize
			);
			
			return false;
		} else {
			return true;
		}
	}
	
	TypeGuard::TypeGuard(size_t size) :
#ifndef NDEBUG
	m_typeName(nullptr), m_typeHash(0),
#endif
	m_typeSize(size)
	{}
	
	TypeGuard::TypeGuard(const std::type_info &info, size_t size) :
#ifndef NDEBUG
	m_typeName(info.name()), m_typeHash(info.hash_code()),
#endif
	m_typeSize(size)
	{}

	bool TypeGuard::operator==(const TypeGuard &other) const {
#ifndef NDEBUG
		return checkType(other.m_typeName, other.m_typeHash, other.m_typeSize);
#else
		return checkTypeSize(other.m_typeSize);
#endif
	}

	bool TypeGuard::operator!=(const TypeGuard &other) const {
#ifndef NDEBUG
		return !checkType(other.m_typeName, other.m_typeHash, other.m_typeSize);
#else
		return !checkTypeSize(other.m_typeSize);
#endif
	}

	size_t TypeGuard::typeSize() const {
		return m_typeSize;
	}

}
