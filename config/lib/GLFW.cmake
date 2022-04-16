
find_package(glfw3 QUIET)

if (glfw3_FOUND)
    list(APPEND vkcv_libraries glfw)

    message(${vkcv_config_msg} " GLFW    -   " ${glfw3_VERSION})
else()
    use_git_submodule("${vkcv_lib_path}/glfw" glfw_status)

    if (${glfw_status})
        add_subdirectory(${vkcv_lib}/glfw)

        list(APPEND vkcv_libraries glfw)
        list(APPEND vkcv_includes ${vkcv_lib_path}/glfw/include)

        message(${vkcv_config_msg} " GLFW    -   " ${glfw3_VERSION})
    endif ()
endif ()
