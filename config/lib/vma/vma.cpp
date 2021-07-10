
#ifndef NDEBUG
#define _DEBUG
#endif

#ifdef __MINGW32__

#include <mutex>

class VmaMutex {
public:
	void Lock() { m_Mutex.lock(); }
	void Unlock() { m_Mutex.unlock(); }
private:
	std::mutex m_Mutex;
};

#define VMA_MUTEX VmaMutex

#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) (aligned_alloc((alignment), (size) ))
#define VMA_SYSTEM_FREE(ptr) free(ptr)
#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.hpp"
