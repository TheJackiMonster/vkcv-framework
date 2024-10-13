
function(include_shader shader include_dir source_dir)
	if (NOT EXISTS ${shader})
		message(WARNING "Shader file does not exist: ${shader}")
	else()
		get_filename_component(filename ${shader} NAME)
		file(SIZE ${shader} filesize)
		
		set(include_target_file ${include_dir}/${filename}.hxx)
		set(source_target_file ${source_dir}/${filename}.cxx)
		
		if ((EXISTS ${source_target_file}) AND (EXISTS ${include_target_file}))
			file(TIMESTAMP ${shader} shader_timestamp "%Y-%m-%dT%H:%M:%S")
			file(TIMESTAMP ${source_target_file} source_timestamp "%Y-%m-%dT%H:%M:%S")
			
			string(COMPARE GREATER ${shader_timestamp} ${source_timestamp} shader_update)
		else()
			set(shader_update true)
		endif()
		
		if (shader_update)
			string(TOUPPER ${filename} varname)
			string(REPLACE "." "_" varname ${varname})
			
			set(shader_header "#pragma once\n")
			string(APPEND shader_header "// This file is auto-generated via cmake, so don't touch it!\n")
			string(APPEND shader_header "extern unsigned char ${varname} [${filesize}]\;\n")
			string(APPEND shader_header "extern unsigned int ${varname}_LEN\;\n")
			string(APPEND shader_header "const std::string ${varname}_SHADER (reinterpret_cast<const char*>(${varname}), ${varname}_LEN)\;")
			
			file(WRITE ${include_target_file} ${shader_header})
			
			find_program(xxd_program "xxd")
			
			if (EXISTS ${xxd_program})
				get_filename_component(shader_directory ${shader} DIRECTORY)
				
				add_custom_command(
						OUTPUT ${source_target_file}
						WORKING_DIRECTORY "${shader_directory}"
						COMMAND ${xxd_program} -i -C "${filename}" "${source_target_file}"
						COMMENT "Processing shader into source files: ${shader}"
				)
			else()
				set(shader_source "// This file is auto-generated via cmake, so don't touch it!\n")
				string(APPEND shader_source "unsigned char ${varname}[] = {")
				
				math(EXPR max_fileoffset "${filesize} - 1" OUTPUT_FORMAT DECIMAL)
				
				message(STATUS "Processing shader into source files: ${shader}")
				
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
				
				string(APPEND shader_source "}\;\n")
				string(APPEND shader_source "unsigned int ${varname}_LEN = ${filesize}\;")
				
				file(WRITE ${source_target_file} ${shader_source})
			endif()
		endif()
	endif()
endfunction()
