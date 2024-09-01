
use_git_submodule("${vkcv_camera_lib_path}/OpenXR-SDK" open_xr_status)

if (${open_xr_status})
	list(APPEND vkcv_camera_includes ${vkcv_camera_lib}/OpenXR-SDK/include)

  add_subdirectory(${vkcv_camera_lib}/OpenXR-SDK)
	
	list(APPEND vkcv_camera_definitions XR_USE_GRAPHICS_API_VULKAN)
endif ()
