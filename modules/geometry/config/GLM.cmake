
find_package(glm QUIET)

if (glm_FOUND)
    list(APPEND vkcv_geometry_includes ${GLM_INCLUDE_DIRS})
    list(APPEND vkcv_geometry_libraries glm)
else()
    use_git_submodule("${vkcv_geometry_lib_path}/glm" glm_status)

    if (${glm_status})
        add_subdirectory(${vkcv_geometry_lib}/glm)

        list(APPEND vkcv_geometry_includes ${vkcv_geometry_lib_path}/glm)
        list(APPEND vkcv_geometry_libraries glm)
    endif ()
endif ()

list(APPEND vkcv_geometry_definitions GLM_DEPTH_ZERO_TO_ONE)
list(APPEND vkcv_geometry_definitions GLM_FORCE_LEFT_HANDED)

if ((WIN32) AND (${CMAKE_SIZEOF_VOID_P} MATCHES 4))
    list(APPEND vkcv_geometry_definitions GLM_ENABLE_EXPERIMENTAL)
endif()
