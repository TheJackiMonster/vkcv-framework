
find_package(Vulkan REQUIRED)

if (Vulkan_FOUND)
    if (NOT EXISTS ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.h)
        use_git_submodule("${vkcv_lib_path}/Vulkan-Headers" vulkan_headers_status)
        
        if (${vulkan_headers_status})
            list(APPEND vkcv_includes ${vkcv_lib}/Vulkan-Headers/include)
        endif()
    else()
        list(APPEND vkcv_includes ${Vulkan_INCLUDE_DIR})
    endif()

    if (NOT EXISTS ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.hpp)
        use_git_submodule("${vkcv_lib_path}/Vulkan-Hpp" vulkan_hpp_status)
    
        if (${vulkan_hpp_status})
            list(APPEND vkcv_includes ${vkcv_lib}/Vulkan-Hpp)
        endif()
    else()
        list(APPEND vkcv_includes ${Vulkan_INCLUDE_DIR})
    endif()
    
    list(APPEND vkcv_libraries ${Vulkan_LIBRARIES})

    message(${vkcv_config_msg} " Vulkan  -   ")
endif ()
