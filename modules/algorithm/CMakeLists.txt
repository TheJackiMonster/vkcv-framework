cmake_minimum_required(VERSION 3.16)
project(vkcv_algorithm)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_algorithm_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_algorithm_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_algorithm_sources
		${vkcv_algorithm_include}/vkcv/algorithm/SinglePassDownsampler.hpp
		${vkcv_algorithm_source}/vkcv/algorithm/SinglePassDownsampler.cpp
)

# Setup some path variables to load libraries
set(vkcv_algorithm_lib lib)
set(vkcv_algorithm_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_algorithm_lib})

# Check and load FidelityFX_SPD
include(config/FidelityFX_SPD.cmake)

# adding source files to the project
add_library(vkcv_algorithm ${vkcv_build_attribute} ${vkcv_algorithm_sources})

# link the required libraries to the module
target_link_libraries(vkcv_algorithm ${vkcv_algorithm_libraries} vkcv vkcv_shader_compiler vkcv_camera vkcv_asset_loader)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_algorithm SYSTEM BEFORE PRIVATE ${vkcv_algorithm_includes} ${vkcv_include} ${vkcv_includes} ${vkcv_shader_compiler_include})

# add the own include directory for public headers
target_include_directories(vkcv_algorithm BEFORE PUBLIC ${vkcv_algorithm_include})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_algorithm_include})
	list(APPEND vkcv_modules_libraries vkcv_algorithm)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()
