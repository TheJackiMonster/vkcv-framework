
find_package(glm QUIET)

if (glm_FOUND)
    list(APPEND vkcv_camera_includes ${GLM_INCLUDE_DIRS})
    list(APPEND vkcv_camera_libraries glm)
else()
    if (EXISTS "${vkcv_camera_lib_path}/glm")
        add_subdirectory(${vkcv_camera_lib}/glm)
        
        list(APPEND vkcv_camera_libraries glm)
        
        list(APPEND vkcv_camera_defines GLM_DEPTH_ZERO_TO_ONE)
        list(APPEND vkcv_camera_defines GLM_FORCE_LEFT_HANDED)
    else()
        message(WARNING "GLM is required..! Update the submodules!")
    endif ()
endif ()
