cmake_policy(SET CMP0007 NEW)
cmake_policy(SET CMP0011 NEW)

# guard calling vcvarsall.bat recursively by checking the environment variable
if(DEFINED ENV{VSCMD_VER})
    set(VS_SETUP TRUE)
    return()
endif()

if(NOT VS_VCVARSALL_FULL_PATH)
    find_program(VS_VCVARSALL_FULL_PATH
        NAMES "vcvarsall.bat"
        NO_CACHE
    )
    if(NOT VS_VCVARSALL_FULL_PATH)
        if(NOT VS_INSTALLATION_PATH)
            include("${CMAKE_CURRENT_LIST_DIR}/find-vs.cmake")
        endif()
        if(VS_INSTALLATION_PATH)
            set(VS_VCVARSALL_FULL_PATH "${VS_INSTALLATION_PATH}/VC/Auxiliary/Build/vcvarsall.bat")
            if(NOT EXISTS "${VS_VCVARSALL_FULL_PATH}")
                unset(VS_VCVARSALL_FULL_PATH)
            endif()
        endif()
    endif()
endif()

if(NOT VS_VCVARSALL_FULL_PATH)
    message(FATAL_ERROR "Cannot determine the path to the vcvarsall.bat script")
endif()

if(NOT VS_VCVARSALL_ARGS)
    set(VS_VCVARSALL_ARGS "")

    if(NOT VS_ARCH)
        if(NOT CMAKE_HOST_SYSTEM_PROCESSOR)
            cmake_host_system_information(RESULT CMAKE_HOST_SYSTEM_PROCESSOR QUERY OS_PLATFORM)
        endif()
        if(NOT CMAKE_HOST_SYSTEM_PROCESSOR)
            message(FATAL_ERROR "Cannot infer arch argument without a valid CMAKE_HOST_SYSTEM_PROCESSOR variable")
        endif()
        if(NOT CMAKE_SYSTEM_PROCESSOR OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL CMAKE_SYSTEM_PROCESSOR)
            set(VS_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}")
        else()
            set(VS_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}_${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    endif()

    if(VS_ARCH)
        list(APPEND VS_VCVARSALL_ARGS "${VS_ARCH}")
    endif()
    if(VS_PLATFORM_TYPE)
        list(APPEND VS_VCVARSALL_ARGS "${VS_PLATFORM_TYPE}")
    endif()
    if(VS_WINSDK_VERSION)
        list(APPEND VS_VCVARSALL_ARGS "${VS_WINSDK_VERSION}")
    endif()
    if(VS_VC_VERSION)
        list(APPEND VS_VCVARSALL_ARGS "-vcvars_ver=${VS_VC_VERSION}")
    endif()
    if(VS_SPECTRE_MODE)
        list(APPEND VS_VCVARSALL_ARGS "-vcvars_spectre_libs=${VS_SPECTRE_MODE}")
    endif()
endif()

if(NOT VS_VCVARSALL_ARGS)
    message(FATAL_ERROR "Cannot run vcvarsall.bat with empty args")
endif()

if(VS_VCVARSALL_FULL_PATH AND VS_VCVARSALL_ARGS)
    execute_process(
        COMMAND cmd /C "${VS_VCVARSALL_FULL_PATH}" ${VS_VCVARSALL_ARGS} && SET
        RESULT_VARIABLE _VS_VCVARSALL_RESULT
        OUTPUT_VARIABLE _VS_VCVARSALL_ENVS_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if(_VS_VCVARSALL_RESULT EQUAL 0)
    string(REPLACE ";" "\\;" _VS_VCVARSALL_ENVS_ESCAPE "${_VS_VCVARSALL_ENVS_OUTPUT}")
    string(REGEX REPLACE "[\r\n]+" "|;" _VS_VCVARSALL_ENVS_SPLIT "${_VS_VCVARSALL_ENVS_ESCAPE}")

    list(GET _VS_VCVARSALL_ENVS_SPLIT 0 _FIRST_LINE)
    string(FIND "${_FIRST_LINE}" "[ERROR:vcvarsall.bat]" _ERROR_FOUND_LOC)

    if(_ERROR_FOUND_LOC LESS 0)
        foreach(_LINE ${_VS_VCVARSALL_ENVS_SPLIT})
            string(REGEX MATCH "^([^=]+)=(.*)[|]$" _LINE_MATCH "${_LINE}")
            if(_LINE_MATCH)
                message(DEBUG "${CMAKE_MATCH_1}=${CMAKE_MATCH_2}")
                set(ENV{${CMAKE_MATCH_1}} "${CMAKE_MATCH_2}")
            endif()
        endforeach()
        set(VS_SETUP TRUE)
    endif()
endif()

unset(_VS_VCVARSALL_RESULT)
unset(_VS_VCVARSALL_ENVS_OUTPUT)
unset(_VS_VCVARSALL_ENVS_ESCAPE)
unset(_VS_VCVARSALL_ENVS_SPLIT)
unset(_FIRST_LINE)
unset(_ERROR_FOUND_LOC)
unset(_LINE)
unset(_LINE_MATCH)

message(DEBUG "PATH=$ENV{PATH}")
message(DEBUG "INCLUDE=$ENV{INCLUDE}")
message(DEBUG "LIB=$ENV{LIB}")
message(DEBUG "WINDOWSSDKDIR=$ENV{WINDOWSSDKDIR}")