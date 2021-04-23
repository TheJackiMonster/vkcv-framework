cmake_minimum_required(VERSION 3.16)
project(vkcv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("-- Language: [ C++ " ${CMAKE_CXX_STANDARD} " ]")

set(vkcv_config ${PROJECT_SOURCE_DIR}/config)
set(vkcv_config_ext ${vkcv_config}/ext)

set(vkcv_lib lib)

set(vkcv_source ${PROJECT_SOURCE_DIR}/src)

set(vkcv_flags ${CMAKE_CXX_FLAGS})

include(${vkcv_config}/Sources.cmake)
include(${vkcv_config}/Libraries.cmake)

set(CMAKE_CXX_FLAGS ${vkcv_flags})

message("-- Flags: [ ${CMAKE_CXX_FLAGS} ]")

add_compile_definitions(${vkcv_definitions})

if (vkcv_directories)
	include_directories(${vkcv_directories})
endif()

add_library(vkcv SHARED ${vkcv_sources})

if(MSVC)
  #enable multicore compilation on visual studio
  target_compile_options(vkcv PRIVATE "/MP" "/openmp")

  #set source groups to create proper filters in visual studio
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${vkcv_sources})
endif()

target_include_directories(vkcv SYSTEM BEFORE PRIVATE ${vkcv_includes})
target_link_libraries(vkcv ${vkcv_libraries})

add_subdirectory(projects)
