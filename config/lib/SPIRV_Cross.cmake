find_package(spirv_cross_c_shared QUIET)

if (spirv-cross_FOUND)
    list(APPEND vkcv_libraries spirv-cross-cpp)

    message(${vkcv_config_msg} " SPIRV Cross    - " ${SPIRV_CROSS_VERSION})
else()
    if (EXISTS "${vkcv_lib_path}/SPIRV-Cross")
        set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_SHARED OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_STATIC ON CACHE INTERNAL "")
        set(SPIRV_CROSS_CLI OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE INTERNAL "")
        
        set(SPIRV_CROSS_ENABLE_GLSL ON CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_MSL OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_CPP ON CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_C_API OFF CACHE INTERNAL "")
        set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE INTERNAL "")
        
        set(SPIRV_CROSS_SKIP_INSTALL ON CACHE INTERNAL "")
    
        add_subdirectory(${vkcv_lib}/SPIRV-Cross)

        list(APPEND vkcv_libraries spirv-cross-cpp)
        list(APPEND vkcv_includes ${vkcv_lib}/SPIV-Cross/include)

        message(${vkcv_config_msg} " SPIRV Cross    - " ${SPIRV_CROSS_VERSION})
    else()
        message(WARNING "SPIRV-Cross is required..! Update the submodules!")
    endif ()
endif ()