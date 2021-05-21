cmake_minimum_required(VERSION 3.16)
project(first_mesh)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# this should fix the execution path to load local files from the project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# adding source files to the project
add_executable(first_mesh src/main.cpp)

# this should fix the execution path to load local files from the project (for MSVC)
if(MSVC)
	set_target_properties(first_triangle PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	set_target_properties(first_triangle PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# including headers of dependencies and the VkCV framework
target_include_directories(first_mesh SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes} ${vkcv_asset_loader_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(first_mesh vkcv ${vkcv_libraries} vkcv_asset_loader ${vkcv_asset_loader_libraries})
