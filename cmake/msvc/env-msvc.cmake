cmake_policy(SET CMP0007 NEW)
cmake_policy(SET CMP0011 NEW)

set(_CMAKE_ARGS "")
foreach(_INDEX RANGE "${CMAKE_ARGC}")
    list(APPEND _CMAKE_ARGS "${CMAKE_ARGV${_INDEX}}")
endforeach()
set(_CMAKE_UNPARSED_SEP "--")
list(FIND _CMAKE_ARGS "${_CMAKE_UNPARSED_SEP}" _CMAKE_UNPARSED_SEP_LOC)
math(EXPR _CMAKE_UNPARSED_BEGIN "${_CMAKE_UNPARSED_SEP_LOC}+1")
math(EXPR _CMAKE_UNPARSED_LENGTH "${CMAKE_ARGC}-${_CMAKE_UNPARSED_SEP_LOC}")
list(SUBLIST _CMAKE_ARGS "${_CMAKE_UNPARSED_BEGIN}" "${_CMAKE_UNPARSED_LENGTH}" _CMAKE_UNPARSED_ARGS)

if(NOT VS_SETUP)
    include("${CMAKE_CURRENT_LIST_DIR}/setup-msvc-vars.cmake")
endif()

if(VS_SETUP)
    message(STATUS "Running command with MSVC env vars")
else()
    message(STATUS "Running command without MSVC env vars")
endif()

execute_process(
    COMMAND ${_CMAKE_UNPARSED_ARGS}
)