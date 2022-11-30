cmake_minimum_required(VERSION 3.16)
project(particle_simulation)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# adding source files to the project
add_project(particle_simulation
		src/main.cpp
		src/ParticleSystem.hpp 
		src/ParticleSystem.cpp
		src/Particle.hpp 
		src/Particle.cpp)

# including headers of dependencies and the VkCV framework
target_include_directories(particle_simulation SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_camera_include}
		${vkcv_shader_compiler_include}
		${vkcv_effects_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(particle_simulation
		vkcv
		vkcv_camera
		vkcv_shader_compiler
		vkcv_effects)
