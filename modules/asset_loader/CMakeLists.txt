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

# Setup some path variables to load libraries
set(vkcv_asset_loader_lib lib)
set(vkcv_asset_loader_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_asset_loader_lib})

# Check and load NLOHMANN_JSON
if (EXISTS "${vkcv_asset_loader_lib_path}/json")
	set(JSON_BuildTests OFF CACHE INTERNAL "")
	
	add_subdirectory(${vkcv_asset_loader_lib}/json)
	
	list(APPEND vkcv_asset_loader_libraries nlohmann_json::nlohmann_json)
else()
	message(WARNING "NLOHMANN_JSON is required..! Update the submodules!")
endif ()

# Check and load FX-GLTF
if (EXISTS "${vkcv_asset_loader_lib_path}/fx-gltf")
	set(FX_GLTF_INSTALL OFF CACHE INTERNAL "")
	set(FX_GLTF_USE_INSTALLED_DEPS OFF CACHE INTERNAL "")
	set(BUILD_TESTING OFF CACHE INTERNAL "")
	
	add_subdirectory(${vkcv_asset_loader_lib}/fx-gltf)
	
	list(APPEND vkcv_asset_loader_libraries fx-gltf)
else()
	message(WARNING "FX-GLTF is required..! Update the submodules!")
endif ()

# adding source files to the module
add_library(vkcv_asset_loader STATIC ${vkcv_asset_loader_sources})

# link the required libraries to the module
target_link_libraries(vkcv_asset_loader ${vkcv_asset_loader_libraries})

# add the own include directory for public headers
target_include_directories(vkcv_asset_loader BEFORE PUBLIC ${vkcv_asset_loader_include})
