cmake_minimum_required(VERSION 3.16)
project(vkcv_asset_loader)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_asset_loader_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_asset_loader_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_asset_loader_sources
		${vkcv_asset_loader_include}/vkcv/asset/asset_loader.hpp
		${vkcv_asset_loader_source}/vkcv/asset/asset_loader.cpp
)

# adding source files to the module
add_library(vkcv_asset_loader STATIC ${vkcv_asset_loader_sources})

# Setup some path variables to load libraries
set(vkcv_asset_loader_lib lib)
set(vkcv_asset_loader_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_asset_loader_lib})

# Check and load NLOHMANN_JSON
include(config/NLOHMANN_JSON.cmake)

# Check and load FX-GLTF
include(config/FX_GLTF.cmake)

# Check and load STB
include(config/STB.cmake)

# link the required libraries to the module
target_link_libraries(vkcv_asset_loader ${vkcv_asset_loader_libraries})

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_asset_loader SYSTEM BEFORE PRIVATE ${vkcv_asset_loader_includes})

# add the own include directory for public headers
target_include_directories(vkcv_asset_loader BEFORE PUBLIC ${vkcv_asset_loader_include})
