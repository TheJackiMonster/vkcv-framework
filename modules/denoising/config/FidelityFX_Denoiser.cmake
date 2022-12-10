
use_git_submodule("${vkcv_denoising_lib_path}/FidelityFX-Denoiser" ffx_denoiser_status)

if (${ffx_denoiser_status})
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-shadows-dnsr/ffx_denoiser_shadows_filter.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-shadows-dnsr/ffx_denoiser_shadows_prepare.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-shadows-dnsr/ffx_denoiser_shadows_tileclassification.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-shadows-dnsr/ffx_denoiser_shadows_util.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-reflection-dnsr/ffx_denoiser_reflections_common.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-reflection-dnsr/ffx_denoiser_reflections_config.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-reflection-dnsr/ffx_denoiser_reflections_prefilter.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-reflection-dnsr/ffx_denoiser_reflections_reproject.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	include_shader(${vkcv_denoising_lib_path}/FidelityFX-Denoiser/ffx-reflection-dnsr/ffx_denoiser_reflections_resolve_temporal.h ${vkcv_denoising_include} ${vkcv_denoising_source})
	
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_shadows_filter.h.cxx)
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_shadows_prepare.h.cxx)
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_shadows_tileclassification.h.cxx)
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_shadows_util.h.cxx)
	
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_reflections_common.h.cxx)
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_reflections_config.h.cxx)
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_reflections_prefilter.h.cxx)
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_reflections_reproject.h.cxx)
	list(APPEND vkcv_denoising_sources ${vkcv_denoising_source}/ffx_denoiser_reflections_resolve_temporal.h.cxx)
endif ()
