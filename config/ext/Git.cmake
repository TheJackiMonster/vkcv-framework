
function(init_git_lfs)
    find_program(git_program "git")

    if (EXISTS ${git_program})
        add_custom_target(
                init_git_lfs
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        )

        add_custom_command(
                TARGET init_git_lfs
                PRE_BUILD
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                COMMAND git lfs install
                COMMENT "Initializing Git LFS"
        )
    endif()
endfunction()

function(init_git_submodules)
    find_program(git_program "git")

    if (EXISTS ${git_program})
        add_custom_target(
                init_git_submodules
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        )

        add_custom_command(
                TARGET init_git_submodules
                PRE_BUILD
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                COMMAND git submodule init
                COMMENT "Initializing Git Submodules"
        )
    endif()
endfunction()

function(use_git_submodule submodule_path submodule_status)
    if (EXISTS "${submodule_path}")
        set(${submodule_status} TRUE PARENT_SCOPE)
        return()
    endif()

    get_filename_component(submodule_directory ${submodule_path} DIRECTORY)
    get_filename_component(submodule_name ${submodule_path} NAME)

    find_program(git_program "git")

    if (EXISTS ${git_program})
        add_custom_target(
                "git_submodule_${submodule_name}"
                WORKING_DIRECTORY "${submodule_directory}"
        )

        add_custom_command(
                TARGET "git_submodule_${submodule_name}"
                PRE_BUILD
                WORKING_DIRECTORY "${submodule_directory}"
                COMMAND git submodule update
                COMMENT "Updating Git Submodules"
        )

        if (EXISTS "${submodule_path}")
            set(${submodule_status} TRUE PARENT_SCOPE)
            return()
        endif()
    endif()

    message(WARNING "${submodule_name} is required..! Update the submodules!")

    set(${submodule_status} FALSE PARENT_SCOPE)
endfunction()
