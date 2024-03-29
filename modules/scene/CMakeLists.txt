cmake_minimum_required(VERSION 3.16)
project(vkcv_scene)

# setting c++ standard for the module
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_scene_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_scene_include ${PROJECT_SOURCE_DIR}/include)

# Add source and header files to the module
set(vkcv_scene_sources
		${vkcv_scene_include}/vkcv/scene/Bounds.hpp
		${vkcv_scene_source}/vkcv/scene/Bounds.cpp
		
		${vkcv_scene_include}/vkcv/scene/Frustum.hpp
		${vkcv_scene_source}/vkcv/scene/Frustum.cpp
		
		${vkcv_scene_include}/vkcv/scene/MeshPart.hpp
		${vkcv_scene_source}/vkcv/scene/MeshPart.cpp
		
		${vkcv_scene_include}/vkcv/scene/Mesh.hpp
		${vkcv_scene_source}/vkcv/scene/Mesh.cpp

		${vkcv_scene_include}/vkcv/scene/Node.hpp
		${vkcv_scene_source}/vkcv/scene/Node.cpp
		
		${vkcv_scene_include}/vkcv/scene/Scene.hpp
		${vkcv_scene_source}/vkcv/scene/Scene.cpp
)

filter_headers(vkcv_scene_sources ${vkcv_scene_include} vkcv_scene_headers)

# adding source files to the module
add_library(vkcv_scene ${vkcv_build_attribute} ${vkcv_scene_sources})
set_target_properties(vkcv_scene PROPERTIES PUBLIC_HEADER "${vkcv_scene_headers}")

# link the required libraries to the module
target_link_libraries(vkcv_scene vkcv)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_scene SYSTEM BEFORE PRIVATE
		${vkcv_include}
		${vkcv_includes}
		${vkcv_asset_loader_include}
		${vkcv_material_include}
		${vkcv_camera_include}
		${vkcv_algorithm_include})

# add the own include directory for public headers
target_include_directories(vkcv_scene BEFORE PUBLIC ${vkcv_scene_include})

# linking with libraries from all dependencies and the VkCV framework
target_link_libraries(vkcv_scene
		vkcv
		vkcv_asset_loader
		vkcv_material
		vkcv_camera
		vkcv_algorithm)

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_scene_include})
	list(APPEND vkcv_modules_libraries vkcv_scene)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv_scene PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv/scene)
