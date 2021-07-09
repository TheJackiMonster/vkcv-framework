
if (EXISTS "${vkcv_lib_path}/VulkanMemoryAllocator")
	add_subdirectory(${vkcv_lib}/VulkanMemoryAllocator)
	
	list(APPEND vkcv_libraries VulkanMemoryAllocator)
	list(APPEND vkcv_includes ${vkcv_lib}/VulkanMemoryAllocator/include)
	
	message(${vkcv_config_msg} " VMA     - ")
else()
	message(WARNING "VulkanMemoryAllocator is required..! Update the submodules!")
endif ()
