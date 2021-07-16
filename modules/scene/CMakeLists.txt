cmake_minimum_required(VERSION 3.16)
project(vkcv_scene)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_scene_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_scene_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_scene_sources
		${vkcv_scene_include}/vkcv/scene/Bounds.hpp
		${vkcv_scene_source}/vkcv/scene/Bounds.cpp
		
		${vkcv_scene_source}/vkcv/scene/Frustum.hpp
		${vkcv_scene_source}/vkcv/scene/Frustum.cpp
		
		${vkcv_scene_include}/vkcv/scene/MeshPart.hpp
		${vkcv_scene_source}/vkcv/scene/MeshPart.cpp
		
		${vkcv_scene_include}/vkcv/scene/Mesh.hpp
		${vkcv_scene_source}/vkcv/scene/Mesh.cpp

		${vkcv_scene_include}/vkcv/scene/Meshlet.hpp
		
		${vkcv_scene_include}/vkcv/scene/Node.hpp
		${vkcv_scene_source}/vkcv/scene/Node.cpp
		
		${vkcv_scene_include}/vkcv/scene/Scene.hpp
		${vkcv_scene_source}/vkcv/scene/Scene.cpp
)

# adding source files to the module
add_library(vkcv_scene STATIC ${vkcv_scene_sources})

# link the required libraries to the module
target_link_libraries(vkcv_scene vkcv)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_scene SYSTEM BEFORE PRIVATE ${vkcv_include} ${vkcv_includes} ${vkcv_asset_loader_include} ${vkcv_material_include} ${vkcv_camera_include})

# add the own include directory for public headers
target_include_directories(vkcv_scene BEFORE PUBLIC ${vkcv_scene_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(vkcv_scene vkcv vkcv_asset_loader vkcv_material vkcv_camera)
