
use_git_submodule("${vkcv_shader_compiler_lib_path}/lz4" lz4_status)

if (${lz4_status})
  set(LZ4_BUNDLED_MODE ON)
	add_subdirectory(${vkcv_shader_compiler_lib}/lz4/build/cmake SYSTEM)

  if(MSVC)
		target_compile_options(
			lz4_static
			PRIVATE /wd5045 /wd4820 /wd4711 /wd6385 /wd6262
		)
	endif()

	set(lz4_include ${vkcv_shader_compiler_lib}/lz4/lib)
endif ()
