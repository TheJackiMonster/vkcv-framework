
use_git_submodule("${vkcv_shader_compiler_lib_path}/unordered_dense" unordered_dense_status)

if (${unordered_dense_status})
  add_subdirectory(${vkcv_shader_compiler_lib}/unordered_dense SYSTEM)

  set(unordered_dense_include ${vkcv_shader_compiler_lib_path}/unordered_dense/include)
endif ()
