
# the first argument should be the project's target
macro(fix_project)
    # this should fix the execution path to load local files from the project (for MSVC)
    if(MSVC)
        set_target_properties(${ARGV1} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        set_target_properties(${ARGV1} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

        # in addition to setting the output directory, the working directory has to be set
        # by default visual studio sets the working directory to the build directory, when using the debugger
        set_target_properties(${ARGV1} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif()
endmacro()
