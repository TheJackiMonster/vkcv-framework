set(vkcv_config_lib ${vkcv_config}/lib)

set(vkcv_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_lib})

if(NOT WIN32)
	set(vkcv_libraries stdc++fs)
	
	# optimization for loading times
	list(APPEND vkcv_flags -pthread)
	list(APPEND vkcv_flags -fopenmp)
endif()

set(vkcv_config_msg " - Library: ")

include(${vkcv_config_lib}/GLFW.cmake)    # glfw-x11 / glfw-wayland					# libglfw3-dev
include(${vkcv_config_lib}/Vulkan.cmake)  # vulkan-intel / vulkan-radeon / nvidia	# libvulkan-dev

if (vkcv_flags)
    list(REMOVE_DUPLICATES vkcv_flags)
endif()

if (vkcv_includes)
    list(REMOVE_DUPLICATES vkcv_includes)
endif ()

include(${vkcv_config_ext}/CheckLibraries.cmake)

if (vkcv_definitions)
    list(REMOVE_DUPLICATES vkcv_definitions)
endif ()

list(JOIN vkcv_flags " " vkcv_flags)
