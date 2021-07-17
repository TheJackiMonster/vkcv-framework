
function(include_shader shader include_dir source_dir)
	get_filename_component(filename ${shader} NAME)
	file(SIZE ${shader} filesize)
	
	string(TOUPPER ${filename} varname)
	string(REPLACE "." "_" varname ${varname})
	
	set(shader_header "")
	string(APPEND shader_header "#pragma once\n")
	string(APPEND shader_header "extern unsigned char ${varname} [${filesize}]\;\n")
	string(APPEND shader_header "extern unsigned int ${varname}_LEN\;\n")
	string(APPEND shader_header "const std::string ${varname}_SHADER (reinterpret_cast<const char*>(${varname}), ${varname}_LEN)\;")
	
	set(shader_source "")
	string(APPEND shader_source "unsigned char ${varname}[] = {")
	
	math(EXPR max_fileoffset "${filesize} - 1" OUTPUT_FORMAT DECIMAL)
	
	foreach(fileoffset RANGE ${max_fileoffset})
		file(READ ${shader} shader_source_byte OFFSET ${fileoffset} LIMIT 1 HEX)
		
		math(EXPR offset_modulo "${fileoffset} % 12" OUTPUT_FORMAT DECIMAL)
		
		if (${offset_modulo} EQUAL 0)
			string(APPEND shader_source "\n  ")
		endif()
		
		if (${fileoffset} LESS ${max_fileoffset})
			string(APPEND shader_source "0x${shader_source_byte}, ")
		else()
			string(APPEND shader_source "0x${shader_source_byte}\n")
		endif()
	endforeach()
	
	string(APPEND shader_source "}\n")
	string(APPEND shader_source "unsigned int ${varname}_LEN = ${filesize}\;")
	
	file(WRITE ${include_dir}/${filename}.hxx ${shader_header})
	file(WRITE ${source_dir}/${filename}.cxx ${shader_source})
endfunction()
