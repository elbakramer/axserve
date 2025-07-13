cmake_minimum_required(VERSION 3.27)

include_guard(GLOBAL)

block(SCOPE_FOR POLICIES)
cmake_policy(SET CMP0007 NEW)

if(NOT CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    message(DEBUG "Not running in script mode")
    return()
endif()

set(_CMAKE_ARGS)
foreach(_INDEX RANGE "${CMAKE_ARGC}")
    list(APPEND _CMAKE_ARGS "${CMAKE_ARGV${_INDEX}}")
endforeach()

set(_CMAKE_UNPARSED_SEP "--")
list(FIND _CMAKE_ARGS "${_CMAKE_UNPARSED_SEP}" _CMAKE_UNPARSED_SEP_LOC)

if(_CMAKE_UNPARSED_SEP_LOC LESS 0)
    message(DEBUG "No arguments given")
    return()
endif()

math(EXPR _CMAKE_UNPARSED_BEGIN "${_CMAKE_UNPARSED_SEP_LOC}+1")
math(EXPR _CMAKE_UNPARSED_LENGTH "${CMAKE_ARGC}-${_CMAKE_UNPARSED_SEP_LOC}")

list(SUBLIST _CMAKE_ARGS "${_CMAKE_UNPARSED_BEGIN}" "${_CMAKE_UNPARSED_LENGTH}" _CMAKE_UNPARSED_ARGS)

if(NOT _CMAKE_UNPARSED_ARGS)
    message(DEBUG "No arguments given")
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/setup-msvc-vars.cmake")
setup_msvc_vars()

execute_process(
    COMMAND ${_CMAKE_UNPARSED_ARGS}
    COMMAND_ERROR_IS_FATAL ANY
)

endblock()
