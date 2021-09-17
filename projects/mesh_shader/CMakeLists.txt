cmake_minimum_required(VERSION 3.16)
project(mesh_shader)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# this should fix the execution path to load local files from the project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# adding source files to the project
add_executable(mesh_shader src/main.cpp)
fix_project(mesh_shader)

# including headers of dependencies and the VkCV framework
target_include_directories(mesh_shader SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes} ${vkcv_testing_include} ${vkcv_camera_include} ${vkcv_meshlet_include} ${vkcv_shader_compiler_include} ${vkcv_gui_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(mesh_shader vkcv ${vkcv_libraries} vkcv_asset_loader ${vkcv_asset_loader_libraries} vkcv_testing vkcv_camera vkcv_meshlet vkcv_shader_compiler vkcv_gui)