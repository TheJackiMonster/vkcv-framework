cmake_minimum_required(VERSION 3.16)
project(vkcv_testing)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_testing_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_testing_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_testing_sources
		${vkcv_testing_include}/vkcv/testing/Test.hpp
		${vkcv_testing_source}/vkcv/testing/Test.cpp
)

# adding source files to the project
add_library(vkcv_testing STATIC ${vkcv_testing_sources})

# add the own include directory for public headers
target_include_directories(vkcv_testing BEFORE PUBLIC ${vkcv_testing_include})

