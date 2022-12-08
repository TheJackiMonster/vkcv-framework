
function(filter_headers sources include_path headers)
	set(header_list "")
	
	foreach(src_path IN ITEMS ${${sources}})
		string(FIND ${src_path} ${include_path} src_include_dir)
		
		if (${src_include_dir} MATCHES 0)
			list(APPEND header_list ${src_path})
		endif()
	endforeach()
	
	set(${headers} "${header_list}" PARENT_SCOPE)
endfunction()
