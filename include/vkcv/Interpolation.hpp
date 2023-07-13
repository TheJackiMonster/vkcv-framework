#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Interpolation.hpp
 * @brief Structure for interpolation
 */
 
#include <algorithm>
#include <cmath>
#include <functional>

#include "Container.hpp"

namespace vkcv {
	
	template<typename V, typename T = double>
	struct interpolation_function {
		typedef std::function<V(const V&, const V&, T)> type;
	};
	
	template<typename V, typename T = double>
	struct interpolation_state {
		T time;
		V value;
		
		bool operator<(const interpolation_state& other) const {
			return time < other.time;
		}
	};
	
	template<typename V, typename T = double>
	struct interpolation {
	private:
		typename interpolation_function<V, T>::type m_function;
		Vector< interpolation_state<V, T> > m_states;
	
	public:
		interpolation(const typename interpolation_function<V, T>::type& function)
		: m_function(function), m_states() {}
		
		interpolation(const interpolation& other) = default;
		interpolation(interpolation&& other) noexcept = default;
		
		~interpolation() = default;
		
		interpolation& operator=(const interpolation& other) = default;
		interpolation& operator=(interpolation&& other) noexcept = default;
		
		void clear() {
			m_states.clear();
		}
		
		void add(T time, const V& value) {
			interpolation_state<V, T> state;
			state.time = time;
			state.value = value;
			m_states.insert(
					std::lower_bound(m_states.begin(), m_states.end(), state),
					state
			);
		}
		
		V operator()(T time) const {
			interpolation_state<V, T> state;
			state.time = time;
			
			auto end = std::lower_bound(m_states.begin(), m_states.end(), state);
			auto start = end != m_states.begin()? (end - 1) : m_states.begin();
			
			if (end == m_states.end()) {
				end = start;
			}
			
			const T ratio = (time - (*start).time) / ((*end).time - (*start).time);
			
			return m_function(
					(*start).value,
					(*end).value,
					std::clamp<T>(
							ratio,
							static_cast<T>(0),
							static_cast<T>(1)
					)
			);
		}
	
	};
	
	template<typename V, typename T = double>
	interpolation<V, T> linearInterpolation() {
		return interpolation<V, T>([](const V& start, const V& end, T ratio) {
			return start * (static_cast<T>(1) - ratio) + end * ratio;
		});
	}
	
	template<typename V, typename T = double>
	interpolation<V, T> cubicInterpolation() {
		return interpolation<V, T>([](const V& start, const V& end, T ratio) {
			const T r0 = (static_cast<T>(1) - ratio) * (static_cast<T>(1) - ratio);
			const T r1 = ratio * ratio;
			
			return (
				start * r0 +
				start * r0 * ratio * static_cast<T>(2) +
				end * r1 * (static_cast<T>(1) - ratio) * static_cast<T>(2) +
				end * r1
			);
		});
	}
	
	template<typename V, typename T = double>
	interpolation<V, T> cosInterpolation() {
		return interpolation<V, T>([](const V& start, const V& end, T ratio) {
			const T cos_ratio = (static_cast<T>(1) - std::cos(
					ratio * static_cast<T>(M_PI)
			)) / static_cast<T>(2);
			
			return start * (static_cast<T>(1) - cos_ratio) + end * cos_ratio;
		});
	}
	
}