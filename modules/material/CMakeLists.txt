cmake_minimum_required(VERSION 3.16)
project(vkcv_material)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_material_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_material_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_material_sources
		${vkcv_material_include}/vkcv/material/Material.hpp
		${vkcv_material_source}/vkcv/material/Material.cpp
)

filter_headers(vkcv_material_sources ${vkcv_material_include} vkcv_material_headers)

# adding source files to the module
add_library(vkcv_material ${vkcv_build_attribute} ${vkcv_material_sources})
set_target_properties(vkcv_material PROPERTIES PUBLIC_HEADER "${vkcv_material_headers}")

# link the required libraries to the module
target_link_libraries(vkcv_material vkcv ${vkcv_libraries})

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_material SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes})

# add the own include directory for public headers
target_include_directories(vkcv_material BEFORE PUBLIC ${vkcv_material_include})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_material_include})
	list(APPEND vkcv_modules_libraries vkcv_material)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv_material PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv/material)
