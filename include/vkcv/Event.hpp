#pragma once
/**
 * @authors Tobias Frisch, Sebastian Gaida, Josch Morgenstern, Katharina Krämer
 * @file vkcv/Event.hpp
 * @brief Template event struct to synchronize callbacks.
 */

#include <functional>

#ifndef __MINGW32__
#ifdef __NO_SEMAPHORES__
#include <mutex>
#else
#include <semaphore>
#endif
#endif

#include <vector>

namespace vkcv {
	
	/**
	 * @brief Template for a function handle to an event
	 *
	 * @tparam T Event parameter type list
	 */
	template<typename... T>
	struct event_handle {
		uint32_t id;
	};

	/**
	 * @brief Template for an event function
	 *
	 * @tparam T Event parameter type list
	 */
    template<typename... T>
    struct event_function {
        typedef std::function<void(T...)> type;
	
		event_handle<T...> handle;
        type callback;
    };

    /**
     * @brief Template for event handling
     *
     * @tparam T Event parameter type list
     */
    template<typename... T>
    struct event {
    private:
        std::vector< event_function<T...> > m_functions;
        uint32_t m_id_counter;
	
#ifndef __MINGW32__
#ifdef __NO_SEMAPHORES__
		std::mutex m_mutex;
#else
		std::binary_semaphore m_semaphore;
#endif
#endif

    public:

        /**
         * @brief Calls all function handles with the given arguments.
         *
         * @param[in,out] arguments Arguments of the given event
         */
        void operator()(T... arguments) {
			lock();

            for (auto &function : this->m_functions) {
				function.callback(arguments...);
			}
            
            unlock();
        }

        /**
         * @brief Adds a function handle to the event to be called.
         *
         * @param[in] callback Event callback
         * @return Handle of the function
         */
		event_handle<T...> add(typename event_function<T...>::type callback) {
			event_function<T...> function;
			function.handle = { m_id_counter++ };
			function.callback = callback;
            this->m_functions.push_back(function);
            return function.handle;
        }

        /**
         * @brief Removes a function handle of the event.
         *
         * @param handle Handle of the function
         */
        void remove(event_handle<T...> handle) {
            this->m_functions.erase(
					std::remove_if(this->m_functions.begin(), this->m_functions.end(), [&handle](auto function){
						return (handle.id == function.handle.id);
					}),
                    this->m_functions.end()
            );
        }
        
        /**
         * @brief Locks the event so its function handles won't
         * be called until unlocked.
         */
        void lock() {
#ifndef __MINGW32__
#ifdef __NO_SEMAPHORES__
			m_mutex.lock();
#else
			m_semaphore.acquire();
#endif
#endif
        }
	
		/**
		 * @brief Unlocks the event so its function handles can
		 * be called after locking.
		 */
        void unlock() {
#ifndef __MINGW32__
#ifdef __NO_SEMAPHORES__
			m_mutex.unlock();
#else
			m_semaphore.release();
#endif
#endif
        }

        explicit event(bool locked = false)
#ifndef __MINGW32__
#ifndef __NO_SEMAPHORES__
		: m_semaphore(locked? 1 : 0)
#endif
#endif
		{
#ifndef __MINGW32__
#ifdef __NO_SEMAPHORES__
			if (locked) m_mutex.lock();
#endif
#endif
		}

        event(const event &other) = delete;

        event(event &&other) = delete;

        ~event() = default;

        event &operator=(const event &other) = delete;

        event &operator=(event &&other) = delete;
		
    };
	
}
