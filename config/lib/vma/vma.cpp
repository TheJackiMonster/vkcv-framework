
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

// TODO: This is not actually a valid way to do aligned allocations!
#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) (malloc(size))
#else
#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) (aligned_alloc((alignment), (size)))
#endif

#define VMA_SYSTEM_FREE(ptr) free(ptr)
#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.hpp"
