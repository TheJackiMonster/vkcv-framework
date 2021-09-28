cmake_minimum_required(VERSION 3.16)
project(rtx_ambient_occlusion)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# this should fix the execution path to load local files from the project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(rtx_source ${PROJECT_SOURCE_DIR}/src/RTX)

set(rtx_sources
		${rtx_source}/RTX.hpp
		${rtx_source}/RTX.cpp

		${rtx_source}/ASManager.hpp
		${rtx_source}/ASManager.cpp

		${rtx_source}/RTXExtensions.hpp
		${rtx_source}/RTXExtensions.cpp
		)

# adding source files to the project
add_executable(rtx_ambient_occlusion src/main.cpp src/teapot.hpp ${rtx_sources})

# this should fix the execution path to load local files from the project (for MSVC)
if(MSVC)
	set_target_properties(rtx_ambient_occlusion PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	set_target_properties(rtx_ambient_occlusion PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    
    # in addition to setting the output directory, the working directory has to be set
	# by default visual studio sets the working directory to the build directory, when using the debugger
	set_target_properties(rtx_ambient_occlusion PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# including headers of dependencies and the VkCV framework
target_include_directories(rtx_ambient_occlusion SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes} ${vkcv_asset_loader_include} ${vkcv_camera_include} ${vkcv_scene_include} ${vkcv_shader_compiler_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(rtx_ambient_occlusion vkcv ${vkcv_libraries} vkcv_asset_loader ${vkcv_asset_loader_libraries} vkcv_camera vkcv_scene vkcv_shader_compiler)
