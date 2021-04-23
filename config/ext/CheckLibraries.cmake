
list(REMOVE_DUPLICATES vkcv_libraries)

foreach (a_lib IN LISTS vkcv_libraries)
    if (NOT EXISTS "${a_lib}")
        string(REGEX MATCH ^/usr/lib/x86_64-linux-gnu/.*$ vkcv_usr_lib_u_match ${a_lib})

        if (vkcv_usr_lib_u_match)
            string(SUBSTRING ${vkcv_usr_lib_u_match} 26 -1 a_lib)

            list(REMOVE_ITEM vkcv_libraries ${vkcv_usr_lib_u_match})
            list(APPEND vkcv_libraries_ext ${a_lib})
        else ()
            string(REGEX MATCH ^/usr/lib/.*$ vkcv_usr_lib_match ${a_lib})
            string(REGEX MATCH ^/lib/.*$ a_lib_match ${a_lib})

            if (vkcv_usr_lib_match)
                list(REMOVE_ITEM vkcv_libraries ${a_lib})

                string(SUBSTRING ${vkcv_usr_lib_match} 8 -1 a_lib)
                set(a_lib "/usr/lib/x86_64-linux-gnu${a_lib}")

                if (EXISTS ${a_lib})
                    list(APPEND vkcv_libraries_ext ${a_lib})
                endif ()
            elseif (a_lib_match)
                list(REMOVE_ITEM vkcv_libraries ${a_lib})

                string(SUBSTRING ${a_lib_match} 4 -1 a_lib)
                set(a_lib "/usr/lib/x86_64-linux-gnu${a_lib}")

                if (EXISTS ${a_lib})
                    list(APPEND vkcv_libraries_ext ${a_lib})
                endif ()
            else ()
                string(REGEX MATCH ^-l.*$ a_lib_match ${a_lib})

                if (a_lib_match)
                    string(SUBSTRING ${a_lib_match} 2 -1 a_lib)

                    list(REMOVE_ITEM vkcv_libraries ${a_lib_match})
                    list(APPEND vkcv_libraries_ext ${a_lib})
                endif ()
            endif ()
        endif ()
    endif ()
endforeach ()

foreach (a_lib IN LISTS vkcv_libraries_ext)
    list(APPEND vkcv_libraries ${a_lib})
endforeach ()


