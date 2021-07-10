
#include <mutex>

#ifndef NDEBUG
#define _DEBUG
#endif

#ifndef _WIN32

#ifndef _aligned_malloc
#define _aligned_malloc(alignment, size) aligned_alloc((alignment), (size))
#endif

#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.hpp"
