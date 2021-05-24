find_package(spirv_cross_c_shared QUIET)

if (spirv-cross_FOUND)
    list(APPEND vkcv_libraries spirv-cross-cpp)

    message(${vkcv_config_msg} " SPIRV Cross    - " ${SPIRV_CROSS_VERSION})
else()
    if (EXISTS "${vkcv_lib_path}/SPIRV-Cross")
        set(SPIRV_CROSS_CLI OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_C_API OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_SKIP_INSTALL ON CACHE INTERNAL "")
    
        add_subdirectory(${vkcv_lib}/SPIRV-Cross)

        list(APPEND vkcv_libraries spirv-cross-cpp)

        message(${vkcv_config_msg} " SPIRV Cross    - " ${SPIRV_CROSS_VERSION})
    else()
        message(WARNING "SPIRV-Cross is required..! Update the submodules!")
    endif ()
endif ()