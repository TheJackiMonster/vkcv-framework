cmake_minimum_required(VERSION 3.16)
project(vkcv_tone_mapping)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_tone_mapping_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_tone_mapping_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_tone_mapping_sources
		${vkcv_tone_mapping_include}/vkcv/tone/ToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/ToneMapping.cpp
		
		${vkcv_tone_mapping_include}/vkcv/tone/ACESToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/ACESToneMapping.cpp
		
		${vkcv_tone_mapping_include}/vkcv/tone/FilmicToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/FilmicToneMapping.cpp
		
		${vkcv_tone_mapping_include}/vkcv/tone/LottesToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/LottesToneMapping.cpp
		
		${vkcv_tone_mapping_include}/vkcv/tone/ReinhardToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/ReinhardToneMapping.cpp
		
		${vkcv_tone_mapping_include}/vkcv/tone/Reinhard2ToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/Reinhard2ToneMapping.cpp
		
		${vkcv_tone_mapping_include}/vkcv/tone/UchimuraToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/UchimuraToneMapping.cpp
		
		${vkcv_tone_mapping_include}/vkcv/tone/Uncharted2ToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/Uncharted2ToneMapping.cpp
		
		${vkcv_tone_mapping_include}/vkcv/tone/UnrealToneMapping.hpp
		${vkcv_tone_mapping_source}/vkcv/tone/UnrealToneMapping.cpp
)

filter_headers(vkcv_tone_mapping_sources ${vkcv_tone_mapping_include} vkcv_tone_mapping_headers)

# Setup some path variables to load libraries
set(vkcv_tone_mapping_lib lib)
set(vkcv_tone_mapping_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_tone_mapping_lib})

# Check and load glsl-tone-map
include(config/GLSL_TONE_MAP.cmake)

# Add compile definitions depending on the build context of the module
add_compile_definitions(${vkcv_tone_mapping_definitions})

# adding source files to the project
add_library(vkcv_tone_mapping ${vkcv_build_attribute} ${vkcv_tone_mapping_sources})
set_target_properties(vkcv_tone_mapping PROPERTIES PUBLIC_HEADER "${vkcv_tone_mapping_headers}")

# link the required libraries to the module
target_link_libraries(vkcv_tone_mapping
		${vkcv_tone_mapping_libraries}
		vkcv
		vkcv_shader_compiler
)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_tone_mapping SYSTEM BEFORE PRIVATE
		${vkcv_tone_mapping_includes}
		${vkcv_include}
		${vkcv_includes}
		${vkcv_shader_compiler_include}
)

# add the own include directory for public headers
target_include_directories(vkcv_tone_mapping BEFORE PUBLIC ${vkcv_tone_mapping_include})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_tone_mapping_include})
	list(APPEND vkcv_modules_libraries vkcv_tone_mapping)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv_tone_mapping PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv/tone)
