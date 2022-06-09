
find_package(Vulkan REQUIRED)

if (Vulkan_FOUND)
    use_git_submodule("${vkcv_lib_path}/Vulkan-Headers" vulkan_headers_status)
    
    if (${vulkan_headers_status})
        list(APPEND vkcv_includes ${vkcv_lib_path}/Vulkan-Headers/include)
    endif()

    use_git_submodule("${vkcv_lib_path}/Vulkan-Hpp" vulkan_hpp_status)

    if (${vulkan_hpp_status})
        list(APPEND vkcv_includes ${vkcv_lib_path}/Vulkan-Hpp)
    endif()
    
    list(APPEND vkcv_libraries ${Vulkan_LIBRARIES})

    message(${vkcv_config_msg} " Vulkan  -   " ${Vulkan_VERSION})
endif ()
