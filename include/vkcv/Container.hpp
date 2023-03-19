#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Container.hpp
 * @brief A header to define container types for the framework.
 */

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace vkcv {
	
	template<typename K, typename V>
	using Dictionary = std::unordered_map<K, V>;
	
	template<typename T>
	using Vector = std::vector<T>;
	
	template<typename T>
	using Set = std::unordered_set<T>;
	
}
