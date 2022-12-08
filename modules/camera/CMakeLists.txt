cmake_minimum_required(VERSION 3.16)
project(vkcv_camera)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_camera_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_camera_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_camera_sources
		${vkcv_camera_include}/vkcv/camera/Camera.hpp
		${vkcv_camera_source}/vkcv/camera/Camera.cpp
		
		${vkcv_camera_include}/vkcv/camera/CameraManager.hpp
		${vkcv_camera_source}/vkcv/camera/CameraManager.cpp
		
		${vkcv_camera_include}/vkcv/camera/CameraController.hpp
		
		${vkcv_camera_include}/vkcv/camera/PilotCameraController.hpp
		${vkcv_camera_source}/vkcv/camera/PilotCameraController.cpp
		
		${vkcv_camera_include}/vkcv/camera/TrackballCameraController.hpp
		${vkcv_camera_source}/vkcv/camera/TrackballCameraController.cpp
)

filter_headers(vkcv_camera_sources ${vkcv_camera_include} vkcv_camera_headers)

# adding source files to the project
add_library(vkcv_camera ${vkcv_build_attribute} ${vkcv_camera_sources})
set_target_properties(vkcv_camera PROPERTIES PUBLIC_HEADER "${vkcv_camera_headers}")

# Setup some path variables to load libraries
set(vkcv_camera_lib lib)
set(vkcv_camera_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_camera_lib})

target_link_libraries(vkcv_camera PUBLIC
		${vkcv_camera_libraries}
		vkcv
		vkcv_geometry
)

target_include_directories(vkcv_camera SYSTEM BEFORE PRIVATE
		${vkcv_camera_includes}
		${vkcv_include}
		${vkcv_includes}
		${vkcv_geometry_include}
)

# add the own include directory for public headers
target_include_directories(vkcv_camera BEFORE PUBLIC ${vkcv_camera_include} ${vkcv_camera_includes})
target_compile_definitions(vkcv_camera PUBLIC ${vkcv_camera_definitions})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_camera_include})
	list(APPEND vkcv_modules_libraries vkcv_camera)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv_camera PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv/camera)
