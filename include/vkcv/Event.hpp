#pragma once

#include <functional>
#include <mutex>

namespace vkcv {

    template<typename... T>
    struct event_function {
        typedef std::function<void(T...)> type;
    };

    /**
     * template for event handling
     * @tparam T parameter list
     */
    template<typename... T>
    struct event {
    private:
        std::vector<typename event_function<T...>::type> m_handles;
        std::mutex m_mutex;

    public:

        /**
         * calls all function handles with the given arguments
         * @param arguments of the given function
         */
        void operator()(T... arguments) {
			lock();
        	
            for (auto &handle : this->m_handles) {
                handle(arguments...);
            }
            
            unlock();
        }

        /**
         * adds a function handle to the event to be called
         * @param handle of the function
         */
        typename event_function<T...>::type add(typename event_function<T...>::type handle) {
            this->m_handles.push_back(handle);
            return handle;
        }

        /**
         * removes a function handle of the event
         * @param handle of the function
         */
        void remove(typename event_function<T...>::type handle) {
            this->m_handles.erase(
                    remove(this->m_handles.begin(), this->m_handles.end(), handle),
                    this->m_handles.end()
            );
        }
        
        /**
         * locks the event so its function handles won't be called
         */
        void lock() {
        	m_mutex.lock();
        }
	
		/**
		* unlocks the event so its function handles can be called after locking
		*/
        void unlock() {
        	m_mutex.unlock();
        }

        event() = default;

        event(const event &other) = delete;

        event(event &&other) = delete;

        ~event() = default;

        event &operator=(const event &other) = delete;

        event &operator=(event &&other) = delete;
    };
}
