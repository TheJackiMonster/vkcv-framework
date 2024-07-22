
use_git_submodule("${vkcv_shader_compiler_lib_path}/slang" slang_status)

if (${slang_status})
	add_subdirectory(config/slang)
	
	list(APPEND vkcv_shader_compiler_libraries slang)
	list(APPEND vkcv_shader_compiler_includes ${vkcv_shader_compiler_lib})
endif ()
