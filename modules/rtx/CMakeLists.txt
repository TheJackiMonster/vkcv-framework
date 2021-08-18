cmake_minimum_required(VERSION 3.16)
project(vkcv_rtx)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_rtx_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_rtx_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_rtx_sources
		${vkcv_rtx_include}/vkcv/rtx/RTX.hpp
		${vkcv_rtx_source}/vkcv/rtx/RTX.cpp

		${vkcv_rtx_include}/vkcv/rtx/ASManager.hpp
		${vkcv_rtx_source}/vkcv/rtx/ASManager.cpp
)

# adding source files to the project
add_library(vkcv_rtx STATIC ${vkcv_rtx_sources})

# link the required libraries to the module
target_link_libraries(vkcv_rtx vkcv)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_rtx SYSTEM BEFORE PRIVATE ${vkcv_include})

# add the own include directory for public headers
target_include_directories(vkcv_rtx BEFORE PUBLIC ${vkcv_rtx_include})