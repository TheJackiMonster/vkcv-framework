cmake_minimum_required(VERSION 3.16)
project(sph)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# this should fix the execution path to load local files from the project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# adding source files to the project
add_executable(sph
		src/main.cpp
		src/Particle.hpp 
		src/Particle.cpp
		src/PipelineInit.hpp
		src/PipelineInit.cpp)

fix_project(sph)

# including headers of dependencies and the VkCV framework
target_include_directories(sph SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_testing_include}
		${vkcv_camera_include}
		${vkcv_shader_compiler_include}
		${vkcv_effects_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(sph
		vkcv
		vkcv_testing
		vkcv_camera
		vkcv_shader_compiler
		vkcv_effects)
