cmake_minimum_required(VERSION 3.9)

project(slang)

# settings c++ standard for slang
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

set(slang_base external/source/base)

set(slang_dir ../../${vkcv_shader_compiler_lib}/slang)

set(slang_source ${slang_dir}/source)
set(slang_source_compiler_core ${slang_source}/compiler-core)
set(slang_source_core ${slang_source}/core)
set(slang_source_slang ${slang_source}/slang)

file(GLOB slang_sources 
        ${slang_source_compiler_core}/*.cpp
        ${slang_source_core}/*.cpp
        ${slang_source_slang}/*.cpp
)

if (${VMA_STATIC_VULKAN_FUNCTIONS})
	add_library(slang STATIC ${slang_sources})
else()
	add_library(slang SHARED ${slang_sources})
endif()

set_target_properties(
        slang PROPERTIES
		
		CXX_EXTENSIONS OFF
		CXX_STANDARD ${CMAKE_CXX_STANDARD}
		CXX_STANDARD_REQUIRED ON
)

target_include_directories(slang PUBLIC ${slang_base} ${slang_source})

# Only link to Vulkan if static linking is used
if (NOT ${VMA_DYNAMIC_VULKAN_FUNCTIONS})
	target_link_libraries(slang PUBLIC Vulkan::Vulkan)
endif()
