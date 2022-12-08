cmake_minimum_required(VERSION 3.16)
project(ray_tracer)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adding source files to the project
add_project(ray_tracer src/main.cpp "src/scene.hpp")

# including headers of dependencies and the VkCV framework
target_include_directories(ray_tracer SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_asset_loader_include}
		${vkcv_camera_include}
		${vkcv_shader_compiler_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(ray_tracer
		vkcv
		vkcv_asset_loader
		${vkcv_asset_loader_libraries}
		vkcv_camera
		vkcv_shader_compiler)
