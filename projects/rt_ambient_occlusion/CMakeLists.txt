cmake_minimum_required(VERSION 3.16)
project(rt_ambient_occlusion)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adding source files to the project
add_project(rt_ambient_occlusion src/main.cpp)

# including headers of dependencies and the VkCV framework
target_include_directories(rt_ambient_occlusion SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_asset_loader_include}
		${vkcv_camera_include}
		${vkcv_geometry_include}
		${vkcv_scene_include}
		${vkcv_shader_compiler_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(rt_ambient_occlusion
		vkcv ${vkcv_libraries}
		vkcv_asset_loader ${vkcv_asset_loader_libraries}
		vkcv_camera
		vkcv_geometry
		vkcv_scene
		vkcv_shader_compiler)
