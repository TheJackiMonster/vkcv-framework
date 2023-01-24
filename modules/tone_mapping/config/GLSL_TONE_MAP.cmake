
use_git_submodule("${vkcv_tone_mapping_lib_path}/glsl-tone-map" glsl_tone_map_status)

if (${glsl_tone_map_status})
	include_shader(${vkcv_tone_mapping_lib_path}/glsl-tone-map/aces.glsl
			${vkcv_tone_mapping_include}
			${vkcv_tone_mapping_source})
	
	include_shader(${vkcv_tone_mapping_lib_path}/glsl-tone-map/filmic.glsl
			${vkcv_tone_mapping_include}
			${vkcv_tone_mapping_source})
	
	include_shader(${vkcv_tone_mapping_lib_path}/glsl-tone-map/lottes.glsl
			${vkcv_tone_mapping_include}
			${vkcv_tone_mapping_source})
	
	include_shader(${vkcv_tone_mapping_lib_path}/glsl-tone-map/reinhard.glsl
			${vkcv_tone_mapping_include}
			${vkcv_tone_mapping_source})
	
	include_shader(${vkcv_tone_mapping_lib_path}/glsl-tone-map/reinhard2.glsl
			${vkcv_tone_mapping_include}
			${vkcv_tone_mapping_source})
	
	include_shader(${vkcv_tone_mapping_lib_path}/glsl-tone-map/uchimura.glsl
			${vkcv_tone_mapping_include}
			${vkcv_tone_mapping_source})
	
	include_shader(${vkcv_tone_mapping_lib_path}/glsl-tone-map/uncharted2.glsl
			${vkcv_tone_mapping_include}
			${vkcv_tone_mapping_source})
	
	include_shader(${vkcv_tone_mapping_lib_path}/glsl-tone-map/unreal.glsl
			${vkcv_tone_mapping_include}
			${vkcv_tone_mapping_source})
	
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_source}/aces.glsl.cxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_source}/filmic.glsl.cxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_source}/lottes.glsl.cxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_source}/reinhard.glsl.cxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_source}/reinhard2.glsl.cxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_source}/uchimura.glsl.cxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_source}/uncharted2.glsl.cxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_source}/unreal.glsl.cxx)
	
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_include}/aces.glsl.hxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_include}/filmic.glsl.hxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_include}/lottes.glsl.hxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_include}/reinhard.glsl.hxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_include}/reinhard2.glsl.hxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_include}/uchimura.glsl.hxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_include}/uncharted2.glsl.hxx)
	list(APPEND vkcv_tone_mapping_sources ${vkcv_tone_mapping_include}/unreal.glsl.hxx)
endif()
