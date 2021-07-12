
#ifndef NDEBUG
#define _DEBUG
#endif

#ifndef _MSVC_LANG
#ifdef __MINGW32__
#include <stdlib.h>

class VmaMutex {
public:
	VmaMutex() : m_locked(false) {}
	
	void Lock() {
		while (m_locked);
		m_locked = true;
	}
	
	void Unlock() {
		m_locked = false;
	}
private:
	bool m_locked;
};

#define VMA_MUTEX VmaMutex

template <typename T>
T* custom_overestimate_malloc(size_t size) {
	return new T[size + (sizeof(T) - 1) / sizeof(T)];
}

void* custom_aligned_malloc(size_t alignment, size_t size) {
	if (alignment > 4) {
		return custom_overestimate_malloc<u_int64_t>(size);
	} else
	if (alignment > 2) {
		return custom_overestimate_malloc<u_int32_t>(size);
	} else
	if (alignment > 1) {
		return custom_overestimate_malloc<u_int16_t>(size);
	} else {
		return custom_overestimate_malloc<u_int8_t>(size);
	}
}

void custom_free(void *ptr) {
	delete[] reinterpret_cast<u_int8_t*>(ptr);
}

#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) (custom_aligned_malloc(alignment, size))
#define VMA_SYSTEM_FREE(ptr) (custom_free(ptr))
#endif
#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.hpp"
