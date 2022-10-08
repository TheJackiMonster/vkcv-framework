cmake_minimum_required(VERSION 3.16)
project(vkcv_geometry)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_geometry_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_geometry_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_geometry_sources
		${vkcv_geometry_include}/vkcv/geometry/Geometry.hpp
		${vkcv_geometry_source}/vkcv/geometry/Geometry.cpp
		${vkcv_geometry_include}/vkcv/geometry/Volume.hpp
		${vkcv_geometry_source}/vkcv/geometry/Volume.cpp
		${vkcv_geometry_include}/vkcv/geometry/Circular.hpp
		${vkcv_geometry_source}/vkcv/geometry/Circular.cpp
		${vkcv_geometry_include}/vkcv/geometry/Sphere.hpp
		${vkcv_geometry_source}/vkcv/geometry/Sphere.cpp
		${vkcv_geometry_include}/vkcv/geometry/Cuboid.hpp
		${vkcv_geometry_source}/vkcv/geometry/Cuboid.cpp
		${vkcv_geometry_include}/vkcv/geometry/Cylinder.hpp
		${vkcv_geometry_source}/vkcv/geometry/Cylinder.cpp
		${vkcv_geometry_include}/vkcv/geometry/Teapot.hpp
		${vkcv_geometry_source}/vkcv/geometry/Teapot.cpp
)

# adding source files to the project
add_library(vkcv_geometry ${vkcv_build_attribute} ${vkcv_geometry_sources})

# Setup some path variables to load libraries
set(vkcv_geometry_lib lib)
set(vkcv_geometry_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_geometry_lib})

include(config/GLM.cmake)

target_link_libraries(vkcv_geometry PUBLIC
		${vkcv_geometry_libraries}
		vkcv
)

target_include_directories(vkcv_geometry SYSTEM BEFORE PRIVATE
		${vkcv_geometry_includes}
		${vkcv_include}
		${vkcv_includes}
)

# add the own include directory for public headers
target_include_directories(vkcv_geometry BEFORE PUBLIC ${vkcv_geometry_include} ${vkcv_geometry_includes})
target_compile_definitions(vkcv_geometry PUBLIC ${vkcv_geometry_definitions})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_geometry_include})
	list(APPEND vkcv_modules_libraries vkcv_geometry)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()
