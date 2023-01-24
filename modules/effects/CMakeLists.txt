cmake_minimum_required(VERSION 3.16)
project(vkcv_effects)

# setting c++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(vkcv_effects_source ${PROJECT_SOURCE_DIR}/src)
set(vkcv_effects_include ${PROJECT_SOURCE_DIR}/include)

set(vkcv_effects_sources
		${vkcv_effects_include}/vkcv/effects/Effect.hpp
		${vkcv_effects_source}/vkcv/effects/Effect.cpp
		
		${vkcv_effects_include}/vkcv/effects/BloomAndFlaresEffect.hpp
		${vkcv_effects_source}/vkcv/effects/BloomAndFlaresEffect.cpp
		
		${vkcv_effects_include}/vkcv/effects/GammaCorrectionEffect.hpp
		${vkcv_effects_source}/vkcv/effects/GammaCorrectionEffect.cpp
)

filter_headers(vkcv_effects_sources ${vkcv_effects_include} vkcv_effects_headers)

set(vkcv_effects_shaders ${PROJECT_SOURCE_DIR}/shaders)

include_shader(${vkcv_effects_shaders}/bloomDownsample.comp ${vkcv_effects_include} ${vkcv_effects_source})
include_shader(${vkcv_effects_shaders}/bloomFlaresComposite.comp ${vkcv_effects_include} ${vkcv_effects_source})
include_shader(${vkcv_effects_shaders}/bloomUpsample.comp ${vkcv_effects_include} ${vkcv_effects_source})
include_shader(${vkcv_effects_shaders}/gammaCorrection.comp ${vkcv_effects_include} ${vkcv_effects_source})
include_shader(${vkcv_effects_shaders}/lensFlares.comp ${vkcv_effects_include} ${vkcv_effects_source})

list(APPEND vkcv_effects_sources ${vkcv_effects_source}/bloomDownsample.comp.cxx)
list(APPEND vkcv_effects_sources ${vkcv_effects_source}/bloomFlaresComposite.comp.cxx)
list(APPEND vkcv_effects_sources ${vkcv_effects_source}/bloomUpsample.comp.cxx)
list(APPEND vkcv_effects_sources ${vkcv_effects_source}/gammaCorrection.comp.cxx)
list(APPEND vkcv_effects_sources ${vkcv_effects_source}/lensFlares.comp.cxx)

list(APPEND vkcv_effects_sources ${vkcv_effects_include}/bloomDownsample.comp.hxx)
list(APPEND vkcv_effects_sources ${vkcv_effects_include}/bloomFlaresComposite.comp.hxx)
list(APPEND vkcv_effects_sources ${vkcv_effects_include}/bloomUpsample.comp.hxx)
list(APPEND vkcv_effects_sources ${vkcv_effects_include}/gammaCorrection.comp.hxx)
list(APPEND vkcv_effects_sources ${vkcv_effects_include}/lensFlares.comp.hxx)

# adding source files to the project
add_library(vkcv_effects ${vkcv_build_attribute} ${vkcv_effects_sources})
set_target_properties(vkcv_effects PROPERTIES PUBLIC_HEADER "${vkcv_effects_headers}")

# link the required libraries to the module
target_link_libraries(vkcv_effects
		${vkcv_effects_libraries}
		vkcv
		vkcv_shader_compiler
		vkcv_camera
		vkcv_asset_loader)

# including headers of dependencies and the VkCV framework
target_include_directories(vkcv_effects SYSTEM BEFORE PRIVATE
		${vkcv_effects_includes}
		${vkcv_include}
		${vkcv_includes}
		${vkcv_shader_compiler_include}
		${vkcv_camera_include}
		${vkcv_asset_loader_include})

# add the own include directory for public headers
target_include_directories(vkcv_effects BEFORE PUBLIC ${vkcv_effects_include})

if (vkcv_parent_scope)
	list(APPEND vkcv_modules_includes ${vkcv_effects_include})
	list(APPEND vkcv_modules_libraries vkcv_effects)
	
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()

install(TARGETS vkcv_effects PUBLIC_HEADER DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}/vkcv/effects)
