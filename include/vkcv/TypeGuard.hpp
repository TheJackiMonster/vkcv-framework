#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/TypeGuard.hpp
 * @brief Support type safety for classes in debug compilation.
 */

#include <stdlib.h>
#include <typeinfo>

namespace vkcv {
	
	class TypeGuard {
	private:
#ifndef NDEBUG
		const char* m_typeName;
		size_t m_typeHash;
		
		[[nodiscard]]
		bool checkType(const char* name, size_t hash, size_t size) const;
#endif
		size_t m_typeSize;
		
		[[nodiscard]]
		bool checkTypeSize(size_t size) const;
		
	public:
		explicit TypeGuard(size_t size = 0);
		TypeGuard(const std::type_info &info, size_t size);
		
		TypeGuard(const TypeGuard &other) = default;
		TypeGuard(TypeGuard &&other) noexcept;
		
		~TypeGuard() = default;
		
		TypeGuard& operator=(const TypeGuard &other) = default;
		TypeGuard& operator=(TypeGuard &&other) noexcept;
		
		bool operator==(const TypeGuard &other) const;
		bool operator!=(const TypeGuard &other) const;
		
		template<typename T>
		[[nodiscard]]
		bool check() const {
#ifndef NDEBUG
			return checkType(typeid(T).name(), typeid(T).hash_code(), sizeof(T));
#else
			return checkTypeSize(sizeof(T));
#endif
		}
		
		[[nodiscard]]
		size_t typeSize() const;
		
	};
	
	template<typename T>
	TypeGuard typeGuard() {
		static TypeGuard guard (typeid(T), sizeof(T));
		return guard;
	}
	
	template<typename T>
	TypeGuard typeGuard(T) {
		return typeGuard<T>();
	}
	
	template<typename T>
	TypeGuard typeGuard(const T&) {
		return typeGuard<T>();
	}
	
	template<typename T>
	TypeGuard typeGuard(T&&) {
		return typeGuard<T>();
	}

}
