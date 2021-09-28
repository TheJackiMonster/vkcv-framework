cmake_minimum_required(VERSION 3.16)
project(vkcv_gui)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_gui_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_gui_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_gui_sources
		${vkcv_gui_include}/vkcv/gui/GUI.hpp
		${vkcv_gui_source}/vkcv/gui/GUI.cpp
		)

# Setup some path variables to load libraries
set(vkcv_gui_lib lib)
set(vkcv_gui_lib_path ${PROJECT_SOURCE_DIR}/${vkcv_gui_lib})

# Check and load IMGUI
include(config/ImGui.cmake)

# adding source files to the module
add_library(vkcv_gui STATIC ${vkcv_gui_sources} ${vkcv_imgui_sources})

# link the required libraries to the module
target_link_libraries(vkcv_gui ${vkcv_gui_libraries} vkcv ${vkcv_libraries})

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_gui SYSTEM BEFORE PRIVATE ${vkcv_gui_includes} ${vkcv_include} ${vkcv_includes})

# add the own include directory for public headers
target_include_directories(vkcv_gui BEFORE PUBLIC ${vkcv_gui_include} ${vkcv_imgui_includes})

target_compile_definitions(vkcv_gui PUBLIC ${vkcv_gui_defines})
