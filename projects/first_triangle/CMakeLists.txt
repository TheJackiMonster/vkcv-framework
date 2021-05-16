cmake_minimum_required(VERSION 3.16)
project(first_triangle)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vert.spv DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/frag.spv DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# adding source files to the project
add_executable(first_triangle src/main.cpp)

# including headers of dependencies and the VkCV framework
target_include_directories(first_triangle SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(first_triangle vkcv ${vkcv_libraries})
