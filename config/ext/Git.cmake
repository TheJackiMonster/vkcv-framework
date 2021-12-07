
function(use_git_submodule submodule_path submodule_status)
    file(GLOB path_glob "${submodule_path}/*")
    list(LENGTH path_glob glob_len)

    if(glob_len GREATER 0)
        set(${submodule_status} TRUE PARENT_SCOPE)
        return()
    endif()

    get_filename_component(submodule_name ${submodule_path} NAME)

    message(WARNING "${submodule_name} is required..! Update the submodules!")
    set(${submodule_status} FALSE PARENT_SCOPE)
endfunction()
