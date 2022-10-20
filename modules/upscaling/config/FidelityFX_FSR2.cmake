
set(vkcv_upscaling_fsr2_override ON)

if (WIN32)
	set(vkcv_upscaling_fsr2_override OFF)
else()
	find_program(wine_program "wine")
	
	if (EXISTS ${wine_program})
		set(vkcv_upscaling_fsr2_override OFF)
	endif()
endif()

if (vkcv_upscaling_fsr2_override)
	list(APPEND vkcv_upscaling_definitions VKCV_OVERRIDE_FSR2_WITH_FSR1=1)
else()
	use_git_submodule("${vkcv_upscaling_lib_path}/FidelityFX-FSR2" ffx_fsr2_status)
	
	if (${ffx_fsr2_status})
		set(FFX_FSR2_API_DX12 OFF CACHE INTERNAL "")
		set(FFX_FSR2_API_VK ON CACHE INTERNAL "")
		
		add_subdirectory(${vkcv_upscaling_lib}/FidelityFX-FSR2/src/ffx-fsr2-api)
		
		list(APPEND vkcv_upscaling_libraries ${FFX_FSR2_API} ${FFX_FSR2_API_VK})
		
		list(APPEND vkcv_upscaling_includes ${vkcv_upscaling_lib}/FidelityFX-FSR2/src/ffx-fsr2-api)
		list(APPEND vkcv_upscaling_includes ${vkcv_upscaling_lib}/FidelityFX-FSR2/src/ffx-fsr2-api/vk)
	endif ()
endif()