cmake_minimum_required(VERSION 3.16)
project(vkcv_camera)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_camera_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_camera_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_camera_sources
		${vkcv_camera_include}/vkcv/camera/Camera.hpp
		${vkcv_camera_source}/vkcv/camera/Camera.cpp
		${vkcv_camera_include}/vkcv/camera/TrackballCamera.hpp
		${vkcv_camera_source}/vkcv/camera/TrackballCamera.cpp
		${vkcv_camera_include}/vkcv/camera/CameraManager.hpp
		${vkcv_camera_source}/vkcv/camera/CameraManager.cpp
)

# adding source files to the project
add_library(vkcv_camera STATIC ${vkcv_camera_sources})

# Setup some path variables to load libraries
set(vkcv_camera_lib lib)
set(vkcv_camera_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_camera_lib})

include(config/GLM.cmake)

target_link_libraries(vkcv_camera PUBLIC ${vkcv_camera_libraries} vkcv)

target_include_directories(vkcv_camera SYSTEM BEFORE PRIVATE ${vkcv_camera_includes} ${vkcv_include})

# add the own include directory for public headers
target_include_directories(vkcv_camera BEFORE PUBLIC ${vkcv_camera_include} ${vkcv_camera_includes})
