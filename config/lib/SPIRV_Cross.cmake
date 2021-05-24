find_package(spirv_cross_c_shared QUIET)

if (spirv-cross_FOUND)
    list(APPEND vkcv_libraries spirv_cross_c_shared)

    message(${vkcv_config_msg} " SPIRV Cross    - ")
else()
    if (EXISTS "${vkcv_lib_path}/SPIRV-Cross")
        add_subdirectory(${vkcv_lib}/SPIRV-Cross)

        list(APPEND vkcv_libraries spirv_cross_c_shared)

        message(${vkcv_config_msg} " SPIRV Cross    - ")
    else()
        message(WARNING "SPIRV-Cross is required..! Update the submodules!")
    endif ()
endif ()