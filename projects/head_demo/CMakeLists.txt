cmake_minimum_required(VERSION 3.16)
project(head_demo)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adding source files to the project
add_project(head_demo src/main.cpp)

# including headers of dependencies and the VkCV framework
target_include_directories(head_demo SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_asset_loader_include}
		${vkcv_camera_include}
		${vkcv_scene_include}
		${vkcv_shader_compiler_include}
		${vkcv_gui_include}
		${vkcv_effects_include}
		${vkcv_upscaling_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(head_demo
		vkcv
		${vkcv_libraries}
		vkcv_asset_loader
		${vkcv_asset_loader_libraries}
		vkcv_camera vkcv_scene
		vkcv_shader_compiler
		vkcv_gui
		vkcv_effects
		vkcv_upscaling)
