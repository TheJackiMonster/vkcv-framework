#pragma once

#include <functional>

#ifndef __MINGW32__
#include <mutex>
#endif

#include <vector>

namespace vkcv {
	
	template<typename... T>
	struct event_handle {
		uint32_t id;
	};

    template<typename... T>
    struct event_function {
        typedef std::function<void(T...)> type;
	
		event_handle<T...> handle;
        type callback;
    };

    /**
     * template for event handling
     * @tparam T parameter list
     */
    template<typename... T>
    struct event {
    private:
        std::vector< event_function<T...> > m_functions;
        uint32_t m_id_counter;
	
#ifndef __MINGW32__
		std::mutex m_mutex;
#endif

    public:

        /**
         * calls all function handles with the given arguments
         * @param arguments of the given function
         */
        void operator()(T... arguments) {
			lock();

            for (auto &function : this->m_functions) {
				function.callback(arguments...);
			}
            
            unlock();
        }

        /**
         * adds a function handle to the event to be called
         * @param callback of the function
         * @return handle of the function
         */
		event_handle<T...> add(typename event_function<T...>::type callback) {
			event_function<T...> function;
			function.handle = { m_id_counter++ };
			function.callback = callback;
            this->m_functions.push_back(function);
            return function.handle;
        }

        /**
         * removes a function handle of the event
         * @param handle of the function
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
         * locks the event so its function handles won't be called
         */
        void lock() {
#ifndef __MINGW32__
			m_mutex.lock();
#endif
        }
	
		/**
		* unlocks the event so its function handles can be called after locking
		*/
        void unlock() {
#ifndef __MINGW32__
			m_mutex.unlock();
#endif
        }

        explicit event(bool locked = false) {
        	if (locked) {
        		lock();
        	}
        }

        event(const event &other) = delete;

        event(event &&other) = delete;

        ~event() = default;

        event &operator=(const event &other) = delete;

        event &operator=(event &&other) = delete;
    };
}
