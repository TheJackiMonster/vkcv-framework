
if (EXISTS "${vkcv_upscaling_lib_path}/FidelityFX-FSR")
	include_shader(${vkcv_upscaling_lib_path}/FidelityFX-FSR/ffx-fsr/ffx_a.h ${vkcv_upscaling_include} ${vkcv_upscaling_source})
	include_shader(${vkcv_upscaling_lib_path}/FidelityFX-FSR/ffx-fsr/ffx_fsr1.h ${vkcv_upscaling_include} ${vkcv_upscaling_source})
	
	list(APPEND vkcv_upscaling_includes ${vkcv_upscaling_lib}/FidelityFX-FSR/ffx-fsr)
else()
	message(WARNING "FidelityFX-FSR is required..! Update the submodules!")
endif ()
