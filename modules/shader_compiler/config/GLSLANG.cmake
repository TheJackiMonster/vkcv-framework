
use_git_submodule("${vkcv_shader_compiler_lib_path}/glslang" glslang_status)

if (${glslang_status})
	set(SKIP_GLSLANG_INSTALL ON CACHE INTERNAL "")
	set(ENABLE_SPVREMAPPER OFF CACHE INTERNAL "")
	set(ENABLE_GLSLANG_BINARIES OFF CACHE INTERNAL "")
	set(ENABLE_GLSLANG_JS OFF CACHE INTERNAL "")
	set(ENABLE_GLSLANG_WEBMIN OFF CACHE INTERNAL "")
	set(ENABLE_GLSLANG_WEBMIN_DEVEL OFF CACHE INTERNAL "")
	set(ENABLE_EMSCRIPTEN_SINGLE_FILE OFF CACHE INTERNAL "")
	set(ENABLE_EMSCRIPTEN_ENVIRONMENT_NODE OFF CACHE INTERNAL "")
	set(ENABLE_HLSL ON CACHE INTERNAL "")
	set(ENABLE_RTTI OFF CACHE INTERNAL "")
	set(ENABLE_EXCEPTIONS OFF CACHE INTERNAL "")
	set(ENABLE_OPT OFF CACHE INTERNAL "")
	set(ENABLE_PCH OFF CACHE INTERNAL "")
	set(ENABLE_CTEST OFF CACHE INTERNAL "")
	set(USE_CCACHE OFF CACHE INTERNAL "")
	
	set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
	set(BUILD_EXTERNAL OFF CACHE INTERNAL "")
	
	add_subdirectory(${vkcv_shader_compiler_lib}/glslang)
	
	list(APPEND vkcv_shader_compiler_libraries glslang SPIRV)
	list(APPEND vkcv_shader_compiler_includes ${vkcv_shader_compiler_lib})
endif ()
