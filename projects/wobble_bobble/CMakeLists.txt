cmake_minimum_required(VERSION 3.16)
project(wobble_bobble)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adding source files to the project
add_project(wobble_bobble src/main.cpp)

# including headers of dependencies and the VkCV framework
target_include_directories(wobble_bobble SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes} ${vkcv_camera_include} ${vkcv_gui_include} ${vkcv_shader_compiler_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(wobble_bobble vkcv vkcv_camera vkcv_gui vkcv_shader_compiler)
