
use_git_submodule("${vkcv_algorithm_lib_path}/FidelityFX-SPD" ffx_spd_status)

if (${ffx_spd_status})
	include_shader(${vkcv_algorithm_lib_path}/FidelityFX-SPD/ffx-spd/ffx_a.h ${vkcv_algorithm_include} ${vkcv_algorithm_source})
	include_shader(${vkcv_algorithm_lib_path}/FidelityFX-SPD/ffx-spd/ffx_spd.h ${vkcv_algorithm_include} ${vkcv_algorithm_source})
	include_shader(${vkcv_algorithm_lib_path}/FidelityFX-SPD/sample/src/VK/SPDIntegration.glsl ${vkcv_algorithm_include} ${vkcv_algorithm_source})
	include_shader(${vkcv_algorithm_lib_path}/FidelityFX-SPD/sample/src/VK/SPDIntegrationLinearSampler.glsl ${vkcv_algorithm_include} ${vkcv_algorithm_source})

	list(APPEND vkcv_algorithm_includes ${vkcv_algorithm_lib}/FidelityFX-SPD/ffx-spd)
	
	list(APPEND vkcv_algorithm_sources ${vkcv_algorithm_source}/ffx_a.h.cxx)
	list(APPEND vkcv_algorithm_sources ${vkcv_algorithm_source}/ffx_spd.h.cxx)
	list(APPEND vkcv_algorithm_sources ${vkcv_algorithm_source}/SPDIntegration.glsl.cxx)
	list(APPEND vkcv_algorithm_sources ${vkcv_algorithm_source}/SPDIntegrationLinearSampler.glsl.cxx)
	
	list(APPEND vkcv_algorithm_sources ${vkcv_algorithm_include}/ffx_a.h.hxx)
	list(APPEND vkcv_algorithm_sources ${vkcv_algorithm_include}/ffx_spd.h.hxx)
	list(APPEND vkcv_algorithm_sources ${vkcv_algorithm_include}/SPDIntegration.glsl.hxx)
	list(APPEND vkcv_algorithm_sources ${vkcv_algorithm_include}/SPDIntegrationLinearSampler.glsl.hxx)
endif ()
