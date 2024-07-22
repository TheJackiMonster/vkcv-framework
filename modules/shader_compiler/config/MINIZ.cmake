
use_git_submodule("${vkcv_shader_compiler_lib_path}/miniz" miniz_status)

if (${miniz_status})
	add_subdirectory(${vkcv_shader_compiler_lib}/miniz SYSTEM)

  set_property(TARGET miniz PROPERTY POSITION_INDEPENDENT_CODE ON)
  # Work around https://github.com/richgel999/miniz/pull/292
	get_target_property(miniz_c_launcher miniz C_COMPILER_LAUNCHER)
	if(MSVC AND miniz_c_launcher MATCHES "ccache")
    set_property(TARGET miniz PROPERTY C_COMPILER_LAUNCHER)
    set_property(TARGET miniz PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "")
	endif()
endif ()
