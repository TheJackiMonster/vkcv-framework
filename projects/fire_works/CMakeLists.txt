cmake_minimum_required(VERSION 3.16)
project(fire_works)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adding source files to the project
add_project(fire_works
		src/main.cpp)

# including headers of dependencies and the VkCV framework
target_include_directories(fire_works SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_camera_include}
		${vkcv_gui_include}
		${vkcv_shader_compiler_include}
		${vkcv_effects_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(fire_works
		vkcv
		vkcv_camera
		vkcv_gui
		vkcv_shader_compiler
		vkcv_effects)
