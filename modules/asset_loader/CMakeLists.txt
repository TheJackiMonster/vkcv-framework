cmake_minimum_required(VERSION 3.16)
project(vkcv_asset_loader)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_asset_loader_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_asset_loader_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_asset_loader_sources
		${vkcv_asset_loader_include}/vkcv/asset/asset_loader.hpp
		${vkcv_asset_loader_source}/vkcv/asset/asset_loader.cpp
)

set(vkcv_asset_loader_lib lib)
set(vkcv_asset_loader_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_asset_loader_lib})

if (EXISTS "${vkcv_asset_loader_lib_path}/json")
	set(JSON_BuildTests OFF CACHE INTERNAL "")
	
	add_subdirectory(${vkcv_asset_loader_lib}/json)
	
	list(APPEND vkcv_asset_loader_libraries nlohmann_json::nlohmann_json)
else()
	message(WARNING "NLOHMANN_JSON is required..! Update the submodules!")
endif ()

if (EXISTS "${vkcv_asset_loader_lib_path}/fx-gltf")
	set(FX_GLTF_INSTALL OFF)
	set(FX_GLTF_USE_INSTALLED_DEPS OFF)
	set(BUILD_TESTING OFF)
	
	add_subdirectory(${vkcv_asset_loader_lib}/fx-gltf)
	
	list(APPEND vkcv_asset_loader_libraries fx-gltf)
else()
	message(WARNING "FX-GLTF is required..! Update the submodules!")
endif ()

# adding source files to the project
add_library(vkcv_asset_loader STATIC ${vkcv_asset_loader_sources})

target_link_libraries(vkcv_asset_loader ${vkcv_asset_loader_libraries})

# add the own include directory for public headers
target_include_directories(vkcv_asset_loader BEFORE PUBLIC ${vkcv_asset_loader_include})
