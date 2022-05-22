
use_git_submodule("${vkcv_upscaling_lib_path}/NVIDIAImageScaling" nvidia_nis_status)

if (${nvidia_nis_status})
	include_shader(${vkcv_upscaling_lib_path}/NVIDIAImageScaling/NIS/NIS_Scaler.h ${vkcv_upscaling_include} ${vkcv_upscaling_source})
	include_shader(${vkcv_upscaling_lib_path}/NVIDIAImageScaling/NIS/NIS_Main.glsl ${vkcv_upscaling_include} ${vkcv_upscaling_source})
	
	list(APPEND vkcv_upscaling_includes ${vkcv_upscaling_lib}/NVIDIAImageScaling/NIS)
	
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_source}/NIS_Scaler.h.cxx)
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_source}/NIS_Main.glsl.cxx)
	
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_include}/NIS_Scaler.h.hxx)
	list(APPEND vkcv_upscaling_sources ${vkcv_upscaling_include}/NIS_Main.glsl.hxx)
endif ()
