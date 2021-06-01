cmake_minimum_required(VERSION 3.16)
project(vkcv_material)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_material_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_material_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_material_sources
		${vkcv_material_include}/vkcv/material/Material.hpp
		${vkcv_material_source}/vkcv/material/Material.cpp
)

# adding source files to the module
add_library(vkcv_material STATIC ${vkcv_material_sources})

# link the required libraries to the module
target_link_libraries(vkcv_material vkcv)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_material SYSTEM BEFORE PRIVATE ${vkcv_include})

# add the own include directory for public headers
target_include_directories(vkcv_material BEFORE PUBLIC ${vkcv_material_include})
