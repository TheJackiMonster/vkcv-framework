cmake_minimum_required(VERSION 3.16)
project(vkcv)

# settings c++ standard for the framework
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# checking build type and setting up a variable
if (CMAKE_BUILD_TYPE)
	string(TOLOWER "${CMAKE_BUILD_TYPE}" _vkcv_build_type)
	set(vkcv_build_${_vkcv_build_type} 1)
endif()

message("-- Language: [ C++ " ${CMAKE_CXX_STANDARD} " ]")

# setting up different paths
set(vkcv_config ${PROJECT_SOURCE_DIR}/config)
set(vkcv_config_ext ${vkcv_config}/ext)

set(vkcv_lib lib)

set(vkcv_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_include ${PROJECT_SOURCE_DIR}/include)

# initializes compiler flags with defaults
set(vkcv_flags ${CMAKE_CXX_FLAGS})

# enabling warnings in the debug build
if (vkcv_build_debug)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(vkcv_flags ${vkcv_flags} " -Weverything")
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(vkcv_flags ${vkcv_flags} " -Wextra -Wall -pedantic")
	else()
		set(vkcv_flags ${vkcv_flags} " -Wall")
	endif()
endif()

# add source files for compilation
include(${vkcv_config}/Sources.cmake)

# configure everything to use the required dependencies
include(${vkcv_config}/Libraries.cmake)

# set the compiler flags for the framework
set(CMAKE_CXX_FLAGS ${vkcv_flags})

message("-- Flags: [ ${CMAKE_CXX_FLAGS} ]")

# set the compile definitions aka preprocessor variables
add_compile_definitions(${vkcv_definitions})

# create VkCV framework as static library using all source files
add_library(vkcv STATIC ${vkcv_sources})

if(MSVC)
  #enable multicore compilation on visual studio
  target_compile_options(vkcv PRIVATE "/MP" "/openmp")

  #set source groups to create proper filters in visual studio
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${vkcv_sources})
endif()

# add include directories from dependencies as system includes
target_include_directories(vkcv SYSTEM BEFORE PRIVATE ${vkcv_includes})

# add the own include directory for public headers
target_include_directories(vkcv BEFORE PUBLIC ${vkcv_include})

# link the framework using all required libraries
target_link_libraries(vkcv ${vkcv_libraries})

# add sub-projects/examples as targets
add_subdirectory(projects)

# add doxygen as target if installed
include(${vkcv_config}/ext/Doxygen.cmake)
