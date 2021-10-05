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
		src/BloomAndFlares.hpp
		src/BloomAndFlares.cpp
		src/PipelineInit.hpp
		src/PipelineInit.cpp)

# this should fix the execution path to load local files from the project (for MSVC)
if(MSVC)
	set_target_properties(sph PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	set_target_properties(sph PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

	# in addition to setting the output directory, the working directory has to be set
	# by default visual studio sets the working directory to the build directory, when using the debugger
	set_target_properties(sph PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# including headers of dependencies and the VkCV framework
target_include_directories(sph SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes} ${vkcv_testing_include} ${vkcv_camera_include} ${vkcv_shader_compiler_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(sph vkcv vkcv_testing vkcv_camera vkcv_shader_compiler)