find_package(glm REQUIRED)

if (glm_FOUND)

#    list(APPEND vkcv_includes ${GLM_INCLUDE_DIRS})
#    list(APPEND vkcv_libraries glm)
    message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
    message(STATUS ${GLM_INCLUDE_DIR})
    message(STATUS ${GLM_INCLUDE_DIRS})
    message(STATUS ${GLM_LIBRARIES})
    message(${vkcv_config_msg} " GLM    -   " ${glm_VERSION})
else()
    if (EXISTS "${vkcv_lib_path}/glfw")
        add_subdirectory(${vkcv_lib}/glfw)

        list(APPEND vkcv_libraries glfw)

        message(${vkcv_config_msg} " GLFW    -   " ${glfw3_VERSION})
    else()
        message(WARNING "GLFW is required..! Update the submodules!")
    endif ()
endif ()