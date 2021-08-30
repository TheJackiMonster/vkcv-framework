cmake_minimum_required(VERSION 3.16)
project(vkcv_compute)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_compute_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_compute_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_compute_sources
        src/compute.cpp
)

# adding source files to the module
add_library(vkcv_compute STATIC ${vkcv_compute_sources})

# Setup some path variables to load libraries
set(vkcv_compute_lib lib)
set(vkcv_compute_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_compute_lib})

# Check and load GLSLANG
include(config/NNEF-Tools.cmake)

# link the required libraries to the module
target_link_libraries(vkcv_compute ${vkcv_compute_libraries} vkcv)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_compute SYSTEM BEFORE PRIVATE ${vkcv_compute_includes} ${vkcv_include})

# add the own include directory for public headers
target_include_directories(vkcv_compute BEFORE PUBLIC ${vkcv_compute_include})
