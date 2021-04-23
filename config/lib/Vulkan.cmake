
find_package(Vulkan REQUIRED)

if (Vulkan_FOUND)
    list(APPEND vkcv_includes ${Vulkan_INCLUDE_DIR})
    list(APPEND vkcv_libraries ${Vulkan_LIBRARIES})

    message(${vkcv_config_msg} " Vulkan  -   ")
endif ()
