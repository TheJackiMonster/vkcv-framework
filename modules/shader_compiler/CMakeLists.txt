cmake_minimum_required(VERSION 3.16)
project(vkcv_shader_compiler)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_shader_compiler_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_shader_compiler_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_shader_compiler_sources
		${vkcv_shader_compiler_include}/vkcv/shader/Compiler.hpp
		${vkcv_shader_compiler_source}/vkcv/shader/Compiler.cpp
		
		${vkcv_shader_compiler_include}/vkcv/shader/GLSLCompiler.hpp
		${vkcv_shader_compiler_source}/vkcv/shader/GLSLCompiler.cpp
)

# adding source files to the module
add_library(vkcv_shader_compiler ${vkcv_build_attribute} ${vkcv_shader_compiler_sources})

# Setup some path variables to load libraries
set(vkcv_shader_compiler_lib lib)
set(vkcv_shader_compiler_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_shader_compiler_lib})

# Check and load GLSLANG
include(config/GLSLANG.cmake)

# link the required libraries to the module
target_link_libraries(vkcv_shader_compiler ${vkcv_shader_compiler_libraries} vkcv)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_shader_compiler SYSTEM BEFORE PRIVATE ${vkcv_shader_compiler_includes} ${vkcv_include} ${vkcv_includes})

# add the own include directory for public headers
target_include_directories(vkcv_shader_compiler BEFORE PUBLIC ${vkcv_shader_compiler_include})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_shader_compiler_include})
	list(APPEND vkcv_modules_libraries vkcv_shader_compiler)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()
