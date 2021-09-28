cmake_minimum_required(VERSION 3.16)
project(vkcv_upscaling)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_upscaling_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_upscaling_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_upscaling_sources
		${vkcv_upscaling_include}/vkcv/upscaling/Upscaling.hpp
		${vkcv_upscaling_source}/vkcv/upscaling/Upscaling.cpp
		
		${vkcv_upscaling_include}/vkcv/upscaling/BilinearUpscaling.hpp
		${vkcv_upscaling_source}/vkcv/upscaling/BilinearUpscaling.cpp
		
		${vkcv_upscaling_include}/vkcv/upscaling/FSRUpscaling.hpp
		${vkcv_upscaling_source}/vkcv/upscaling/FSRUpscaling.cpp
)

# Setup some path variables to load libraries
set(vkcv_upscaling_lib lib)
set(vkcv_upscaling_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_upscaling_lib})

# Check and load FidelityFX_FSR
include(config/FidelityFX_FSR.cmake)

# adding source files to the project
add_library(vkcv_upscaling STATIC ${vkcv_upscaling_sources})

# link the required libraries to the module
target_link_libraries(vkcv_upscaling ${vkcv_upscaling_libraries} vkcv vkcv_shader_compiler)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_upscaling SYSTEM BEFORE PRIVATE ${vkcv_upscaling_includes} ${vkcv_include} ${vkcv_shader_compiler_include})

# add the own include directory for public headers
target_include_directories(vkcv_upscaling BEFORE PUBLIC ${vkcv_upscaling_include})
