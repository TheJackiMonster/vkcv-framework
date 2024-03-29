cmake_minimum_required(VERSION 3.16)
project(voxelization)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adding source files to the project
add_project(voxelization src/main.cpp)

target_sources(voxelization PRIVATE
    src/Voxelization.hpp
    src/Voxelization.cpp
    src/ShadowMapping.hpp
    src/ShadowMapping.cpp)

# including headers of dependencies and the VkCV framework
target_include_directories(voxelization SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_asset_loader_include}
		${vkcv_camera_include}
		${vkcv_shader_compiler_include}
		${vkcv_tone_mapping_include}
		${vkcv_gui_include}
		${vkcv_upscaling_include}
		${vkcv_effects_include}
		${vkcv_algorithm_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(voxelization
		vkcv
		${vkcv_libraries}
		vkcv_asset_loader
		${vkcv_asset_loader_libraries}
		vkcv_camera
		vkcv_shader_compiler
		vkcv_tone_mapping
		vkcv_gui
		vkcv_upscaling
		vkcv_effects
		vkcv_algorithm)
