cmake_minimum_required(VERSION 3.16)
project(vkcv_denoising)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_denoising_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_denoising_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_denoising_sources
		${vkcv_denoising_include}/vkcv/denoising/Denoiser.hpp
		${vkcv_denoising_source}/vkcv/denoising/Denoiser.cpp
		
		${vkcv_denoising_include}/vkcv/denoising/ShadowDenoiser.hpp
		${vkcv_denoising_source}/vkcv/denoising/ShadowDenoiser.cpp
		
		${vkcv_denoising_include}/vkcv/denoising/ReflectionDenoiser.hpp
		${vkcv_denoising_source}/vkcv/denoising/ReflectionDenoiser.cpp
)

filter_headers(vkcv_denoising_sources ${vkcv_denoising_include} vkcv_denoising_headers)

# Setup some path variables to load libraries
set(vkcv_denoising_lib lib)
set(vkcv_denoising_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_denoising_lib})

# Check and load FidelityFX_Denoiser
include(config/FidelityFX_Denoiser.cmake)

# Add compile definitions depending on the build context of the module
add_compile_definitions(${vkcv_denoising_definitions})

set(vkcv_denoising_shaders ${PROJECT_SOURCE_DIR}/shaders)

include_shader(${vkcv_denoising_shaders}/reflectionDenoiser_prefilter.hlsl ${vkcv_denoising_include} ${vkcv_denoising_source})
include_shader(${vkcv_denoising_shaders}/reflectionDenoiser_reproject.hlsl ${vkcv_denoising_include} ${vkcv_denoising_source})
include_shader(${vkcv_denoising_shaders}/reflectionDenoiser_resolveTemporal.hlsl ${vkcv_denoising_include} ${vkcv_denoising_source})
include_shader(${vkcv_denoising_shaders}/shadowDenoiser_filter.hlsl ${vkcv_denoising_include} ${vkcv_denoising_source})
include_shader(${vkcv_denoising_shaders}/shadowDenoiser_prepare.hlsl ${vkcv_denoising_include} ${vkcv_denoising_source})
include_shader(${vkcv_denoising_shaders}/shadowDenoiser_tileClassification.hlsl ${vkcv_denoising_include} ${vkcv_denoising_source})

list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/reflectionDenoiser_prefilter.hlsl.cxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/reflectionDenoiser_reproject.hlsl.cxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/reflectionDenoiser_resolveTemporal.hlsl.cxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/shadowDenoiser_filter.hlsl.cxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/shadowDenoiser_prepare.hlsl.cxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/shadowDenoiser_tileClassification.hlsl.cxx)

list(APPEND vkcv_denoising_sources ${vkcv_denoising_include}/reflectionDenoiser_prefilter.hlsl.hxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_include}/reflectionDenoiser_reproject.hlsl.hxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_include}/reflectionDenoiser_resolveTemporal.hlsl.hxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_include}/shadowDenoiser_filter.hlsl.hxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_include}/shadowDenoiser_prepare.hlsl.hxx)
list(APPEND vkcv_denoising_sources ${vkcv_denoising_include}/shadowDenoiser_tileClassification.hlsl.hxx)

# adding source files to the project
add_library(vkcv_denoising ${vkcv_build_attribute} ${vkcv_denoising_sources})
set_target_properties(vkcv_denoising PROPERTIES PUBLIC_HEADER "${vkcv_denoising_headers}")

# link the required libraries to the module
target_link_libraries(vkcv_denoising
		${vkcv_denoising_libraries}
		vkcv
		vkcv_shader_compiler
)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_denoising SYSTEM BEFORE PRIVATE
		${vkcv_denoising_includes}
		${vkcv_include}
		${vkcv_includes}
		${vkcv_shader_compiler_include}
)

# add the own include directory for public headers
target_include_directories(vkcv_denoising BEFORE PUBLIC ${vkcv_denoising_include})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_denoising_include})
	list(APPEND vkcv_modules_libraries vkcv_denoising)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv_denoising PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv/denoising)
