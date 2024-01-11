
use_git_submodule("${vkcv_shader_compiler_lib_path}/murmur3" murmur3_status)

if (${murmur3_status})
    add_library(murmur3 STATIC ${vkcv_shader_compiler_lib}/murmur3/murmur3.c)

    target_include_directories(murmur3 INTERFACE ${vkcv_shader_compiler_lib}/murmur3)
    set_target_properties(murmur3 PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif ()
