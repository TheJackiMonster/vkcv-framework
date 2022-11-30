cmake_minimum_required(VERSION 3.16)
project(vkcv_asset_loader)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_asset_loader_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_asset_loader_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_asset_loader_sources
		${vkcv_asset_loader_include}/vkcv/asset/asset_loader.hpp
		${vkcv_asset_loader_source}/vkcv/asset/asset_loader.cpp
)

filter_headers(vkcv_asset_loader_sources ${vkcv_asset_loader_include} vkcv_asset_loader_headers)

# adding source files to the module
add_library(vkcv_asset_loader ${vkcv_build_attribute} ${vkcv_asset_loader_sources})
set_target_properties(vkcv_asset_loader PROPERTIES PUBLIC_HEADER "${vkcv_asset_loader_headers}")

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
target_link_libraries(vkcv_asset_loader ${vkcv_asset_loader_libraries} vkcv ${vkcv_libraries})

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_asset_loader SYSTEM BEFORE PRIVATE ${vkcv_asset_loader_includes}  ${vkcv_include} ${vkcv_includes})

# add the own include directory for public headers
target_include_directories(vkcv_asset_loader BEFORE PUBLIC ${vkcv_asset_loader_include})

target_compile_definitions(vkcv_asset_loader PUBLIC ${vkcv_asset_loader_definitions})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_asset_loader_include})
	list(APPEND vkcv_modules_libraries vkcv_asset_loader)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv_asset_loader PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv/asset)
