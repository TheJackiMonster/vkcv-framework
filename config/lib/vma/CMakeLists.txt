cmake_minimum_required(VERSION 3.9)

project(VulkanMemoryAllocator)

find_package(Vulkan REQUIRED)

option(VMA_HPP_PATH "Location of C++ headers" "")

option(VMA_VULKAN_H_PATH "Location of Vulkan C headers" "")
option(VMA_VULKAN_HPP_PATH "Location of Vulkan C++ headers" "")

message(STATUS "VMA_BUILD_SAMPLE = ${VMA_BUILD_SAMPLE}")
message(STATUS "VMA_BUILD_SAMPLE_SHADERS = ${VMA_BUILD_SAMPLE_SHADERS}")
message(STATUS "VMA_BUILD_REPLAY = ${VMA_BUILD_REPLAY}")

option(VMA_RECORDING_ENABLED "Enable VMA memory recording for debugging" OFF)
option(VMA_USE_STL_CONTAINERS "Use C++ STL containers instead of VMA's containers" OFF)
option(VMA_STATIC_VULKAN_FUNCTIONS "Link statically with Vulkan API" OFF)
option(VMA_DYNAMIC_VULKAN_FUNCTIONS "Fetch pointers to Vulkan functions internally (no static linking)" ON)
option(VMA_DEBUG_ALWAYS_DEDICATED_MEMORY "Every allocation will have its own memory block" OFF)
option(VMA_DEBUG_INITIALIZE_ALLOCATIONS "Automatically fill new allocations and destroyed allocations with some bit pattern" OFF)
option(VMA_DEBUG_GLOBAL_MUTEX "Enable single mutex protecting all entry calls to the library" OFF)
option(VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT "Never exceed VkPhysicalDeviceLimits::maxMemoryAllocationCount and return error" OFF)

message(STATUS "VMA_RECORDING_ENABLED = ${VMA_RECORDING_ENABLED}")
message(STATUS "VMA_USE_STL_CONTAINERS = ${VMA_USE_STL_CONTAINERS}")
message(STATUS "VMA_DYNAMIC_VULKAN_FUNCTIONS = ${VMA_DYNAMIC_VULKAN_FUNCTIONS}")
message(STATUS "VMA_DEBUG_ALWAYS_DEDICATED_MEMORY = ${VMA_DEBUG_ALWAYS_DEDICATED_MEMORY}")
message(STATUS "VMA_DEBUG_INITIALIZE_ALLOCATIONS = ${VMA_DEBUG_INITIALIZE_ALLOCATIONS}")
message(STATUS "VMA_DEBUG_GLOBAL_MUTEX = ${VMA_DEBUG_GLOBAL_MUTEX}")
message(STATUS "VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT = ${VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT}")

if (${VMA_STATIC_VULKAN_FUNCTIONS})
	add_library(VulkanMemoryAllocator STATIC vma.cpp)
else()
	add_library(VulkanMemoryAllocator SHARED vma.cpp)
endif()

set_target_properties(
		VulkanMemoryAllocator PROPERTIES
		
		CXX_EXTENSIONS OFF
		# Use C++14
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
)

target_include_directories(VulkanMemoryAllocator SYSTEM BEFORE PRIVATE ${VMA_VULKAN_H_PATH} ${VMA_VULKAN_HPP_PATH})
target_include_directories(VulkanMemoryAllocator PUBLIC ${VMA_HPP_PATH})

# Only link to Vulkan if static linking is used
if (NOT ${VMA_DYNAMIC_VULKAN_FUNCTIONS})
	target_link_libraries(VulkanMemoryAllocator PUBLIC Vulkan::Vulkan)
endif()

target_compile_definitions(
		VulkanMemoryAllocator
		
		PUBLIC
		VMA_USE_STL_CONTAINERS=$<BOOL:${VMA_USE_STL_CONTAINERS}>
		VMA_DYNAMIC_VULKAN_FUNCTIONS=$<BOOL:${VMA_DYNAMIC_VULKAN_FUNCTIONS}>
		VMA_DEBUG_ALWAYS_DEDICATED_MEMORY=$<BOOL:${VMA_DEBUG_ALWAYS_DEDICATED_MEMORY}>
		VMA_DEBUG_INITIALIZE_ALLOCATIONS=$<BOOL:${VMA_DEBUG_INITIALIZE_ALLOCATIONS}>
		VMA_DEBUG_GLOBAL_MUTEX=$<BOOL:${VMA_DEBUG_GLOBAL_MUTEX}>
		VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT=$<BOOL:${VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT}>
		VMA_RECORDING_ENABLED=$<BOOL:${VMA_RECORDING_ENABLED}>
)
