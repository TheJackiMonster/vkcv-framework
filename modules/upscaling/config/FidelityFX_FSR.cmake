
if (EXISTS "${vkcv_upscaling_lib_path}/FidelityFX-FSR")
	include_shader(${vkcv_upscaling_lib_path}/FidelityFX-FSR/ffx-fsr/ffx_a.h ${vkcv_upscaling_include} ${vkcv_upscaling_source})
	include_shader(${vkcv_upscaling_lib_path}/FidelityFX-FSR/ffx-fsr/ffx_fsr1.h ${vkcv_upscaling_include} ${vkcv_upscaling_source})
	include_shader(${vkcv_upscaling_lib_path}/FidelityFX-FSR/sample/src/VK/FSR_Pass.glsl ${vkcv_upscaling_include} ${vkcv_upscaling_source})
	
	list(APPEND vkcv_upscaling_includes ${vkcv_upscaling_lib}/FidelityFX-FSR/ffx-fsr)
	
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_source}/ffx_a.h.cxx)
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_source}/ffx_fsr1.h.cxx)
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_source}/FSR_Pass.glsl.cxx)
	
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_include}/ffx_a.h.hxx)
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_include}/ffx_fsr1.h.hxx)
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_include}/FSR_Pass.glsl.hxx)
else()
	message(WARNING "FidelityFX-FSR is required..! Update the submodules!")
endif ()
