
use_git_submodule("${vkcv_shader_compiler_lib_path}/shady" shady_status)

if (${shady_status})
    set(EXTERNAL_SPIRV_HEADERS ON CACHE INTERNAL "")
    set(EXTERNAL_MURMUR3 ON CACHE INTERNAL "")

	add_subdirectory(${vkcv_shader_compiler_lib}/shady)
	
	list(APPEND vkcv_shader_compiler_libraries shady)
	list(APPEND vkcv_shader_compiler_includes ${vkcv_shader_compiler_lib}/shady/include)
endif ()
