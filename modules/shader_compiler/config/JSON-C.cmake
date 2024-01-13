
use_git_submodule("${vkcv_shader_compiler_lib_path}/json-c" json_c_status)

if (${json_c_status})
	add_subdirectory(${vkcv_shader_compiler_lib}/json-c)
	
	list(APPEND vkcv_shader_compiler_libraries json-c)
	list(APPEND vkcv_shader_compiler_includes ${vkcv_shader_compiler_lib})
endif ()
