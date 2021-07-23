cmake_minimum_required(VERSION 3.16)
project(vkcv_meshlet)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_meshlet_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_meshlet_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_meshlet_sources
		${vkcv_meshlet_include}/vkcv/meshlet/Meshlet.hpp
		${vkcv_meshlet_source}/vkcv/meshlet/Meshlet.cpp
		${vkcv_meshlet_include}/vkcv/meshlet/Tipsify.hpp
		${vkcv_meshlet_source}/vkcv/meshlet/Tipsify.cpp
)

# adding source files to the module
add_library(vkcv_meshlet STATIC ${vkcv_meshlet_sources})


# link the required libraries to the module
target_link_libraries(vkcv_meshlet vkcv ${vkcv_libraries})

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_meshlet SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes} ${vkcv_asset_loader_include} ${vkcv_camera_include})

# add the own include directory for public headers
target_include_directories(vkcv_meshlet BEFORE PUBLIC ${vkcv_meshlet_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(vkcv_meshlet vkcv vkcv_asset_loader vkcv_camera)
