
#ifndef NDEBUG
#define _DEBUG
#endif

#ifndef _MSVC_LANG
#include <stdlib.h>

#ifdef __MINGW32__
class VmaMutex {
public:
	void Lock() {} // TODO: This should actually lock!
	void Unlock() {} // TODO: This should actually unlock!
};

#define VMA_MUTEX VmaMutex
#endif

#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) (aligned_alloc((alignment), (size)))
#define VMA_SYSTEM_FREE(ptr) free(ptr)
#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.hpp"
