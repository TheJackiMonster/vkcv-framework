
if (EXISTS "${vkcv_compute_lib_path}/NNEF-Tools")

	add_subdirectory(${vkcv_compute_lib}/NNEF-Tools/parser/cpp)
	
	list(APPEND vkcv_compute_libraries nnef)
	list(APPEND vkcv_compute_includes ${vkcv_compute_lib}/NNEF-Tools/parser/cpp/include)
else()
	message(WARNING "NNEF-Tools is required..! Update the submodules!")
endif ()
