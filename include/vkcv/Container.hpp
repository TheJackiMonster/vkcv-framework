#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Container.hpp
 * @brief A header to define container types for the framework.
 */

#include <vector>

#ifdef VKCV_UNORDERED_CONTAINER
#include <unordered_map>
#include <unordered_set>
#else
#include <map>
#include <set>
#endif

namespace vkcv {
	
	template<typename T>
	using Vector = std::vector<T>;

#ifdef VKCV_UNORDERED_CONTAINER
	
	template<typename K, typename V>
	using Dictionary = std::unordered_map<K, V>;

	template<typename T>
	using Set = std::unordered_set<T>;

#else
	
	template<typename K, typename V>
	using Dictionary = std::map<K, V>;
	
	template<typename T>
	using Set = std::set<T>;

#endif

}
