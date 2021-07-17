
if (EXISTS "${vkcv_upscaling_lib_path}/FidelityFX-FSR")
	list(APPEND vkcv_upscaling_includes ${vkcv_upscaling_lib}/FidelityFX-FSR/ffx-fsr)
else()
	message(WARNING "FidelityFX-FSR is required..! Update the submodules!")
endif ()
