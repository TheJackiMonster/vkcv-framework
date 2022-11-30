cmake_minimum_required(VERSION 3.16)
project(vkcv_meshlet)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_meshlet_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_meshlet_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_meshlet_sources
		${vkcv_meshlet_include}/vkcv/meshlet/Meshlet.hpp
		${vkcv_meshlet_source}/vkcv/meshlet/Meshlet.cpp

		${vkcv_meshlet_include}/vkcv/meshlet/Tipsify.hpp
		${vkcv_meshlet_source}/vkcv/meshlet/Tipsify.cpp

		${vkcv_meshlet_include}/vkcv/meshlet/Forsyth.hpp
		${vkcv_meshlet_source}/vkcv/meshlet/Forsyth.cpp
)

filter_headers(vkcv_meshlet_sources ${vkcv_meshlet_include} vkcv_meshlet_headers)

# adding source files to the module
add_library(vkcv_meshlet ${vkcv_build_attribute} ${vkcv_meshlet_sources})
set_target_properties(vkcv_meshlet PROPERTIES PUBLIC_HEADER "${vkcv_meshlet_headers}")

# link the required libraries to the module
target_link_libraries(vkcv_meshlet vkcv ${vkcv_libraries})

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_meshlet SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes} ${vkcv_asset_loader_include} ${vkcv_camera_include})

# add the own include directory for public headers
target_include_directories(vkcv_meshlet BEFORE PUBLIC ${vkcv_meshlet_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(vkcv_meshlet vkcv vkcv_asset_loader vkcv_camera)

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_meshlet_include})
	list(APPEND vkcv_modules_libraries vkcv_meshlet)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv_meshlet PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv/meshlet)
