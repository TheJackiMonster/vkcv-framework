
use_git_submodule("${vkcv_shader_compiler_lib_path}/SPIRV-Headers" spriv_headers_status)

if (${spriv_headers_status})
	add_subdirectory(${vkcv_shader_compiler_lib}/SPIRV-Headers)
	
	list(APPEND vkcv_shader_compiler_libraries SPIRV-Headers)
	list(APPEND vkcv_shader_compiler_includes ${vkcv_shader_compiler_lib}/SPIRV-Headers/include)
endif ()
