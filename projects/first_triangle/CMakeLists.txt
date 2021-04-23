cmake_minimum_required(VERSION 3.16)
project(first_triangle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(first_triangle src/main.cpp)

target_include_directories(first_triangle SYSTEM BEFORE PRIVATE ${vkcv_source} ${vkcv_includes})
target_link_libraries(first_triangle vkcv ${vkcv_libraries})
