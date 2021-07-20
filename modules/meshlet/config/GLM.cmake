
find_package(glm QUIET)

if (glm_FOUND)
    list(APPEND vkcv_meshlet_includes ${GLM_INCLUDE_DIRS})
    list(APPEND vkcv_meshlet_libraries glm)
else()
    if (EXISTS "${vkcv_meshlet_lib_path}/glm")
        add_subdirectory(${vkcv_meshlet_lib}/glm)

        list(APPEND vkcv_meshlet_includes ${vkcv_meshlet_lib_path}/glm)
        list(APPEND vkcv_meshlet_libraries glm)
    else()
        message(WARNING "GLM is required..! Update the submodules!")
    endif ()
endif ()

list(APPEND vkcv_meshlet_definitions GLM_DEPTH_ZERO_TO_ONE)
list(APPEND vkcv_meshlet_definitions GLM_FORCE_LEFT_HANDED)

if ((WIN32) AND (${CMAKE_SIZEOF_VOID_P} MATCHES 4))
    list(APPEND vkcv_meshlet_definitions GLM_ENABLE_EXPERIMENTAL)
endif()
