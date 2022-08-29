#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/TypeGuard.hpp
 * @brief Support type safety for classes in debug compilation.
 */

#include <stdlib.h>
#include <typeinfo>

namespace vkcv {
	
	/**
	 * @brief Class bringing type safety during runtime to other classes.
	 */
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
		/**
		 * Explicit constructor to create a type guard by a specific
		 * size only. The guard will not verify an individual type but
		 * whether its size matches the requirement.
		 *
		 * @param[in] size Size of type
		 */
		explicit TypeGuard(size_t size = 0);
		
		/**
		 * Explicit constructor to create a type guard by a types
		 * ID information and its size. The guard will verify a type
		 * by all available information in debug mode.
		 *
		 * @param[in] info ID information of type
		 * @param[in] size Size of type
		 */
		TypeGuard(const std::type_info &info, size_t size);
		
		TypeGuard(const TypeGuard &other) = default;
		TypeGuard(TypeGuard &&other) noexcept = default;
		
		~TypeGuard() = default;
		
		TypeGuard& operator=(const TypeGuard &other) = default;
		TypeGuard& operator=(TypeGuard &&other) noexcept = default;
		
		/**
		 * Operator to compare two type guards and returns
		 * whether their stored type information and size
		 * match as boolean value.
		 *
		 * @param[in] other Other type guard
		 * @return True if the details match, otherwise false.
		 */
		bool operator==(const TypeGuard &other) const;
		
		/**
		 * Operator to compare two type guards and returns
		 * whether their stored type information and size
		 * do not match as boolean value.
		 *
		 * @param[in] other Other type guard
		 * @return True if the details differ, otherwise false.
		 */
		bool operator!=(const TypeGuard &other) const;
		
		/**
		 * Checks whether a type from a template parameter
		 * matches with the used type by the given guard.
		 *
		 * @tparam T Type to check against
		 * @return True if both types match, otherwise false.
		 */
		template<typename T>
		[[nodiscard]]
		bool check() const {
#ifndef NDEBUG
			return checkType(typeid(T).name(), typeid(T).hash_code(), sizeof(T));
#else
			return checkTypeSize(sizeof(T));
#endif
		}
		
		/**
		 * Returns the size of this guards type in bytes.
		 *
		 * @return Size of type
		 */
		[[nodiscard]]
		size_t typeSize() const;
		
	};
	
	/**
	 * Creates a new type guard with a given type specified
	 * as template parameter.
	 *
	 * @tparam T Type
	 * @return New type guard
	 */
	template<typename T>
	TypeGuard typeGuard() {
		static TypeGuard guard (typeid(T), sizeof(T));
		return guard;
	}
	
	/**
	 * Creates a new type guard with a given type specified
	 * as template parameter by the passed parameter.
	 *
	 * @tparam T Type
	 * @return New type guard
	 */
	template<typename T>
	TypeGuard typeGuard(T) {
		return typeGuard<T>();
	}
	
	/**
	 * Creates a new type guard with a given type specified
	 * as template parameter by the passed parameter.
	 *
	 * @tparam T Type
	 * @return New type guard
	 */
	template<typename T>
	TypeGuard typeGuard(const T&) {
		return typeGuard<T>();
	}
	
	/**
	 * Creates a new type guard with a given type specified
	 * as template parameter by the passed parameter.
	 *
	 * @tparam T Type
	 * @return New type guard
	 */
	template<typename T>
	TypeGuard typeGuard(T&&) {
		return typeGuard<T>();
	}

}
