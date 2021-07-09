
if (EXISTS "${vkcv_lib_path}/VulkanMemoryAllocator-Hpp")
	set(VMA_HPP_PATH "${vkcv_lib_path}/VulkanMemoryAllocator-Hpp" CACHE INTERNAL "")
	
	set(VMA_RECORDING_ENABLED OFF CACHE INTERNAL "")
	set(VMA_USE_STL_CONTAINERS OFF CACHE INTERNAL "")
	set(VMA_STATIC_VULKAN_FUNCTIONS ON CACHE INTERNAL "")
	set(VMA_DYNAMIC_VULKAN_FUNCTIONS OFF CACHE INTERNAL "")
	set(VMA_DEBUG_ALWAYS_DEDICATED_MEMORY OFF CACHE INTERNAL "")
	set(VMA_DEBUG_INITIALIZE_ALLOCATIONS OFF CACHE INTERNAL "")
	set(VMA_DEBUG_GLOBAL_MUTEX OFF CACHE INTERNAL "")
	set(VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT OFF CACHE INTERNAL "")
	
	add_subdirectory(vma)
	
	list(APPEND vkcv_libraries VulkanMemoryAllocator)
	list(APPEND vkcv_includes ${vkcv_lib_path}/VulkanMemoryAllocator-Hpp)
	
	message(${vkcv_config_msg} " VMA     - ")
else()
	message(WARNING "VulkanMemoryAllocator is required..! Update the submodules!")
endif ()
