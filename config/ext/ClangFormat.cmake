
find_program(clang_format_program "clang-format")

if (EXISTS ${clang_format_program})
	# note the option ALL which allows to format the source together with the application
	add_custom_target( clang_format ALL
			COMMAND ${clang_format_program} -style=file --sort-includes -i ${vkcv_sources}
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "Formatting code with Clang-Format"
			VERBATIM )
else ()
	message(WARNING "Doxygen need to be installed to generate the doxygen documentation")
endif ()
