
find_package(glm QUIET)

if (glm_FOUND)
    list(APPEND vkcv_camera_includes ${GLM_INCLUDE_DIRS})
    list(APPEND vkcv_camera_libraries glm)
else()
    if (EXISTS "${vkcv_camera_lib_path}/glm")
        add_subdirectory(${vkcv_camera_lib}/glm)
        
        list(APPEND vkcv_camera_libraries glm)
    else()
        message(WARNING "GLM is required..! Update the submodules!")
    endif ()
endif ()
