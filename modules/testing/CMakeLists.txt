cmake_minimum_required(VERSION 3.16)
project(vkcv_testing)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_testing_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_testing_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_testing_sources
		${vkcv_testing_include}/vkcv/testing/Test.hpp
		${vkcv_testing_source}/vkcv/testing/Test.cpp
		${vkcv_testing_include}/vkcv/camera/Camera.hpp
		${vkcv_testing_source}/vkcv/camera/Camera.cpp
		src/vkcv/camera/TrackballCamera.cpp)

# adding source files to the project
add_library(vkcv_testing STATIC ${vkcv_testing_sources})

#include(GLM.cmake)                                               # libglm-dev
#include(GLFW.cmake)												 # libglfw3-dev
find_package(glfw3 QUIET)
find_package(glm QUIET)
target_link_libraries(vkcv_testing PUBLIC glm glfw)

# add the own include directory for public headers
target_include_directories(vkcv_testing BEFORE PUBLIC ${vkcv_testing_include})

