cmake_minimum_required(VERSION 3.16)
project(vkcv)

# cmake options
option(BUILD_MODULES "Enables building VkCV as shared libraries" ON)
option(BUILD_PROJECTS "Enables building the VkCV projects" ON)
option(BUILD_CLANG_FORMAT "Enables formatting the source code" OFF)
option(BUILD_DOXYGEN_DOCS "Enables building the VkCV doxygen documentation" OFF)
option(BUILD_SHARED "Enables building VkCV as shared libraries" OFF)
option(BUILD_VMA_VULKAN_VERSION "Enforce a specific Vulkan version for VMA" OFF)
option(BUILD_VALIDATION_FORCED "Enforce validation layers being built-in" OFF)

# uncomment the following line if cmake will refuse to build projects
#set(BUILD_PROJECTS ON)

if ((WIN32) AND (NOT BUILD_VMA_VULKAN_VERSION))
	set(BUILD_VMA_VULKAN_VERSION "1.3.0")
endif()

if (BUILD_PROJECTS)
	set(BUILD_MODULES ${BUILD_PROJECTS})
endif()

message(STATUS "Options:")
message(" - BUILD_MODULES: ${BUILD_MODULES}")
message(" - BUILD_PROJECTS: ${BUILD_PROJECTS}")
message(" - BUILD_CLANG_FORMAT: ${BUILD_CLANG_FORMAT}")
message(" - BUILD_DOXYGEN_DOCS: ${BUILD_DOXYGEN_DOCS}")
message(" - BUILD_SHARED: ${BUILD_SHARED}")

if (BUILD_SHARED)
	set(vkcv_build_attribute SHARED)
else()
	set(vkcv_build_attribute STATIC)
endif()

# settings c++ standard for the framework
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# checking build type and setting up a variable
if (CMAKE_BUILD_TYPE)
	string(TOLOWER "${CMAKE_BUILD_TYPE}" _vkcv_build_type)
	set(vkcv_build_${_vkcv_build_type} 1)
endif()

if (EXISTS "/usr/bin/mold")
	set(CMAKE_LINKER "/usr/bin/mold")
endif()

message(STATUS "Language: [ C++ " ${CMAKE_CXX_STANDARD} " ]")
message(STATUS "Compiler: [ " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION} " ]")
message(STATUS "Linker: [ " ${CMAKE_LINKER} " ]")

if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0.0"))
	message(FATAL_ERROR "Upgrade your compiler! GCC 9.0+ is required!")
elseif(MINGW)
	message(WARNING "MinGW is impressively unstable! So beware it may not compile or crash during runtime!")
endif()

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
		#set(vkcv_flags ${vkcv_flags} " -Weverything")
		set(vkcv_flags ${vkcv_flags} " -Wextra -Wall -Wno-unused-parameter")
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(vkcv_flags ${vkcv_flags} " -Wextra -Wall -pedantic -Wno-unused-parameter")
	else()
		set(vkcv_flags ${vkcv_flags} " -W4")
	endif()
	
	list(APPEND vkcv_definitions VULKAN_VALIDATION_LAYERS)
elseif (BUILD_VALIDATION_FORCED)
	list(APPEND vkcv_definitions VULKAN_VALIDATION_LAYERS)
endif()

if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0.0"))
	list(APPEND vkcv_definitions __NO_SEMAPHORES__)
endif()

if (((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "4.0.0")) OR
	((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12.0.0")))
	list(APPEND vkcv_definitions VKCV_UNORDERED_CONTAINER)
endif()

# configure everything to use the required dependencies
include(${vkcv_config}/Libraries.cmake)

# set macro to enable vulkan debug labels
list(APPEND vkcv_definitions VULKAN_DEBUG_LABELS)

# set the compile definitions aka preprocessor variables
add_compile_definitions(${vkcv_definitions})

# check if the framework is used from a parent scope
get_directory_property(vkcv_parent_scope PARENT_DIRECTORY)

if (BUILD_MODULES)
	message(STATUS "Modules: ON")
	
	# add modules as targets
	add_subdirectory(modules)
else()
	message(STATUS "Modules: OFF")
endif()

# add source files for compilation
include(${vkcv_config}/Sources.cmake)

message(STATUS "Framework:")
message(" - Includes: [ ${vkcv_includes} ]")
message(" - Libraries: [ ${vkcv_libraries} ]")
message(" - Flags: [ ${vkcv_flags} ]")
message(" - Headers: [ ${vkcv_headers} ]")

# set the compiler flags for the framework
set(CMAKE_CXX_FLAGS ${vkcv_flags})

# create VkCV framework as library using all source files
add_library(vkcv ${vkcv_build_attribute} ${vkcv_sources})
set_target_properties(vkcv PROPERTIES PUBLIC_HEADER "${vkcv_headers}")

if(MSVC)
  #enable multicore compilation on visual studio
  target_compile_options(vkcv PRIVATE "/MP" "/openmp" "/Zc:offsetof-")

  #set source groups to create proper filters in visual studio
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${vkcv_sources})
endif()

# add include directories from dependencies as system includes
target_include_directories(vkcv SYSTEM BEFORE PRIVATE ${vkcv_includes})

# add the own include directory for public headers
target_include_directories(vkcv BEFORE PUBLIC ${vkcv_include})

# link the framework using all required libraries
target_link_libraries(vkcv ${vkcv_libraries})

if (BUILD_PROJECTS)
	message(STATUS "Projects: ON")
	
	# add sub-projects/examples as targets
	add_subdirectory(projects)
else()
	message(STATUS "Projects: OFF")
endif()

if (BUILD_DOXYGEN_DOCS)
	message(STATUS "Doxygen: ON")
	
	# add doxygen as target if installed
	include(${vkcv_config}/ext/Doxygen.cmake)
else()
	message(STATUS "Doxygen: OFF")
endif()

if (vkcv_parent_scope)
	list(APPEND vkcv_includes ${vkcv_include})
	list(APPEND vkcv_libraries vkcv)
	
	if (BUILD_MODULES)
		list(APPEND vkcv_includes ${vkcv_modules_includes})
		list(APPEND vkcv_libraries ${vkcv_modules_libraries})
	endif()
	
	set(vkcv_includes ${vkcv_includes} PARENT_SCOPE)
	set(vkcv_libraries ${vkcv_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv)
