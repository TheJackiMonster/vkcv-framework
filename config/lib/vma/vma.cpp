
#ifndef NDEBUG
#define _DEBUG
#endif

#ifndef _MSVC_LANG
#include <stdlib.h>
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
