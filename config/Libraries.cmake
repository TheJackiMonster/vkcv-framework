
set(vkcv_config_lib ${vkcv_config}/lib)
set(vkcv_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_lib})

if(NOT WIN32)
	if (((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.1.0")) OR
		((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0.0")))
		set(vkcv_libraries stdc++fs)
	endif()
	
	if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
		list(APPEND vkcv_flags -Xpreprocessor)
	endif()
	
	# optimization for loading times
	list(APPEND vkcv_flags -pthread)
	list(APPEND vkcv_flags -fopenmp)
endif()

# add custom functions to check for git submodules
include(${vkcv_config_ext}/Git.cmake)

list(APPEND vkcv_definitions _USE_MATH_DEFINES)

# some formatted printing
set(vkcv_config_msg " - Library: ")

# load dependencies via separate cmake file
include(${vkcv_config_lib}/GLFW.cmake)    # glfw-x11 / glfw-wayland					# libglfw3-dev
include(${vkcv_config_lib}/Vulkan.cmake)  # vulkan-intel / vulkan-radeon / nvidia	# libvulkan-dev
include(${vkcv_config_lib}/SPIRV_Cross.cmake)  # SPIRV-Cross	                    # libspirv_cross_c_shared
include(${vkcv_config_lib}/VulkanMemoryAllocator.cmake) # VulkanMemoryAllocator

# cleanup of compiler flags
if (vkcv_flags)
    list(REMOVE_DUPLICATES vkcv_flags)
endif()

# cleanup of include directories from dependencies
if (vkcv_includes)
    list(REMOVE_DUPLICATES vkcv_includes)
endif ()

# fix dependencies for different Linux distros (looking at you Ubuntu)
include(${vkcv_config_ext}/CheckLibraries.cmake)

# add custom function to include a file like a shader as string
include(${vkcv_config_ext}/IncludeShader.cmake)

# cleanup of compiler definitions aka preprocessor variables
if (vkcv_definitions)
    list(REMOVE_DUPLICATES vkcv_definitions)
endif ()

list(JOIN vkcv_flags " " vkcv_flags)
