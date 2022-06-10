
set(vkcv_modules_includes)
set(vkcv_modules_libraries)

# Add new modules here:
add_subdirectory(algorithm)
add_subdirectory(asset_loader)
add_subdirectory(camera)
add_subdirectory(effects)
add_subdirectory(gui)
add_subdirectory(material)
add_subdirectory(meshlet)
add_subdirectory(scene)
add_subdirectory(shader_compiler)
add_subdirectory(testing)
add_subdirectory(upscaling)

message(STATUS "Modules:")
message(" - Includes: [ ${vkcv_modules_includes} ]")
message(" - Libraries: [ ${vkcv_modules_libraries} ]")

if (vkcv_parent_scope)
	set(vkcv_modules_includes ${vkcv_modules_includes} PARENT_SCOPE)
	set(vkcv_modules_libraries ${vkcv_modules_libraries} PARENT_SCOPE)
endif()
