
# check if Doxygen is installed
find_package(Doxygen QUIET)

if (DOXYGEN_FOUND)
	# note the option ALL which allows to build the docs together with the application
	add_custom_target( doc_doxygen ALL
			COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM )
else (DOXYGEN_FOUND)
	message(WARNING "Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
