cmake_minimum_required(VERSION 3.27)

include_guard(GLOBAL)

block(SCOPE_FOR POLICIES)
cmake_policy(SET CMP0140 NEW)

function(find_vswhere FIND_VSWHERE_VAR)
    set(OPTIONS
        NO_CACHE
        REQUIRED
    )
    set(ONE_VALUE_KEYWORDS)
    set(MULTI_VALUE_KEYWORDS
        NAMES
        VS_INSTALLER_HOME_PATHS
    )
    cmake_parse_arguments(FIND_VSWHERE
        "${OPTIONS}"
        "${ONE_VALUE_KEYWORDS}"
        "${MULTI_VALUE_KEYWORDS}"
        ${ARGN}
    )
    if(NOT FIND_VSWHERE_NO_CACHE AND DEFINED CACHE{"${FIND_VSWHERE_VAR}"})
        return()
    endif()
    if(NOT FIND_VSWHERE_NAMES)
        set(FIND_VSWHERE_NAMES "vswhere.exe")
    endif()
    if(NOT FIND_VSWHERE_VS_INSTALLER_HOME_PATHS)
        set(PROGRAM_FILES "ProgramFiles")
        set(PROGRAM_FILES_X86 "ProgramFiles(x86)")
        set(FIND_VSWHERE_VS_INSTALLER_HOME_PATHS
            "$ENV{${PROGRAM_FILES}}/Microsoft Visual Studio/Installer"
            "$ENV{${PROGRAM_FILES_X86}}/Microsoft Visual Studio/Installer"
            "$ENV{SystemDrive}/Program Files/Microsoft Visual Studio/Installer"
            "$ENV{SystemDrive}/Program Files (x86)/Microsoft Visual Studio/Installer"
        )
    endif()
    find_program("${FIND_VSWHERE_VAR}"
        NAMES ${FIND_VSWHERE_NAMES}
        HINTS ${FIND_VSWHERE_VS_INSTALLER_HOME_PATHS}
        NO_CACHE "${FIND_VSWHERE_NO_CACHE}"
        REQUIRED "${FIND_VSWHERE_REQUIRED}"
    )
    return(PROPAGATE "${FIND_VSWHERE_VAR}")
endfunction()

function(find_vs FIND_VS_VAR)
    set(OPTIONS
        NO_CACHE
        REQUIRED
    )
    set(ONE_VALUE_KEYWORDS
        VSWHERE_PATH
    )
    set(MULTI_VALUE_KEYWORDS
        PRODUCTS
        REQUIRES
        WORKLOADS
        COMPONENTS
    )
    cmake_parse_arguments(FIND_VS
        "${OPTIONS}"
        "${ONE_VALUE_KEYWORDS}"
        "${MULTI_VALUE_KEYWORDS}"
        ${ARGN}
    )
    if(NOT FIND_VS_NO_CACHE AND DEFINED CACHE{${FIND_VS_VAR}})
        return()
    endif()
    if(NOT FIND_VS_VSWHERE_PATH)
        find_vswhere(FIND_VS_VSWHERE_PATH)
    endif()
    if(NOT EXISTS "${FIND_VS_VSWHERE_PATH}")
        if(NOT FIND_VS_NO_CACHE)
            set("${FIND_VS_VAR}" "${FIND_VS_VAR}-NOTFOUND" CACHE PATH "")
        else()
            set("${FIND_VS_VAR}" "${FIND_VS_VAR}-NOTFOUND")
        endif()
        return(PROPAGATE "${FIND_VS_VAR}")
    endif()
    if(NOT FIND_VS_PRODUCTS)
        set(FIND_VS_PRODUCTS
            "Microsoft.VisualStudio.Product.BuildTools"
            "Microsoft.VisualStudio.Product.Community"
            "Microsoft.VisualStudio.Product.Professional"
            "Microsoft.VisualStudio.Product.Enterprise"
        )
    endif()
    if(NOT FIND_VS_WORKLOADS)
        set(FIND_VS_WORKLOADS
            "Microsoft.VisualStudio.Workload.VCTools"
        )
    endif()
    if(NOT FIND_VS_COMPONENTS)
        set(FIND_VS_COMPONENTS
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
            "Microsoft.VisualStudio.Component.VC.CMake.Project"
            "Microsoft.VisualStudio.Component.VC.ATL"
            "Microsoft.VisualStudio.Component.VC.ATLMFC"
        )
    endif()
    if(NOT FIND_VS_REQUIRES)
        set(FIND_VS_REQUIRES ${FIND_VS_WORKLOADS} ${FIND_VS_COMPONENTS})
    endif()
    set(FIND_VS_COMMAND
        "${FIND_VS_VSWHERE_PATH}"
        -products ${FIND_VS_PRODUCTS}
        -requires ${FIND_VS_REQUIRES}
        -latest
        -format value
        -property installationPath
        -utf8
    )
    execute_process(
        COMMAND ${FIND_VS_COMMAND}
        OUTPUT_VARIABLE "${FIND_VS_VAR}"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ENCODING UTF8
        COMMAND_ERROR_IS_FATAL ANY
    )
    if(${FIND_VS_VAR})
        file(TO_CMAKE_PATH "${${FIND_VS_VAR}}" "${FIND_VS_VAR}")
    endif()
    if(NOT EXISTS "${${FIND_VS_VAR}}")
        if(FIND_VS_REQUIRED)
            message(FATAL_ERROR "Failed to find Visual Studio")
        endif()
        if(NOT FIND_VS_NO_CACHE)
            set("${FIND_VS_VAR}" "${FIND_VS_VAR}-NOTFOUND" CACHE PATH "")
        else()
            set("${FIND_VS_VAR}" "${FIND_VS_VAR}-NOTFOUND")
        endif()
    endif()
    return(PROPAGATE "${FIND_VS_VAR}")
endfunction()

function(find_vcvarsall FIND_VCVARSALL_VAR)
    set(OPTIONS
        NO_CACHE
        REQUIRED
    )
    set(ONE_VALUE_KEYWORDS)
    set(MULTI_VALUE_KEYWORDS
        NAMES
        VS_HOME_PATHS
    )
    cmake_parse_arguments(FIND_VCVARSALL
        "${OPTIONS}"
        "${ONE_VALUE_KEYWORDS}"
        "${MULTI_VALUE_KEYWORDS}"
        ${ARGN}
    )
    if(NOT FIND_VCVARSALL_NO_CACHE AND DEFINED CACHE{"${FIND_VCVARSALL_VAR}"})
        return()
    endif()
    if(NOT FIND_VCVARSALL_NAMES)
        set(FIND_VCVARSALL_NAMES "vcvarsall.bat")
    endif()
    if(NOT FIND_VCVARSALL_VS_HOME_PATHS)
        find_vs(FIND_VCVARSALL_VS_HOME_PATHS)
    endif()
    list(
        TRANSFORM FIND_VCVARSALL_VS_HOME_PATHS
        APPEND "/VC/Auxiliary/Build/"
        OUTPUT_VARIABLE FIND_VCVARSALL_HINTS
    )
    find_program("${FIND_VCVARSALL_VAR}"
        NAMES ${FIND_VCVARSALL_NAMES}
        HINTS ${FIND_VCVARSALL_HINTS}
        NO_CACHE "${FIND_VCVARSALL_NO_CACHE}"
        REQUIRED "${FIND_VCVARSALL_REQUIRED}"
    )
    return(PROPAGATE "${FIND_VCVARSALL_VAR}")
endfunction()

function(run_vcvarsall)
    set(OPTIONS
        OPTIONAL
    )
    set(ONE_VALUE_KEYWORDS
        ARCH
        PLATFORM_TYPE
        WINSDK_VERSION
        VC_VERSION
        SPECTRE_MODE
        VCVARSALL_PATH
    )
    set(MULTI_VALUE_KEYWORDS)
    cmake_parse_arguments(RUN_VCVARSALL
        "${OPTIONS}"
        "${ONE_VALUE_KEYWORDS}"
        "${MULTI_VALUE_KEYWORDS}"
        ${ARGN}
    )
    if(NOT RUN_VCVARSALL_VCVARSALL_PATH)
        find_vcvarsall(RUN_VCVARSALL_VCVARSALL_PATH REQUIRED)
    endif()
    set(RUN_VCVARSALL_ARGS)
    if(NOT RUN_VCVARSALL_ARCH)
        if(NOT CMAKE_HOST_SYSTEM_PROCESSOR)
            cmake_host_system_information(RESULT CMAKE_HOST_SYSTEM_PROCESSOR QUERY OS_PLATFORM)
        endif()
        if(NOT CMAKE_HOST_SYSTEM_PROCESSOR)
            if(RUN_VCVARSALL_OPTIONAL)
                return()
            endif()
            message(FATAL_ERROR "Cannot infer arch argument without a valid CMAKE_HOST_SYSTEM_PROCESSOR variable")
        endif()
        if(NOT CMAKE_SYSTEM_PROCESSOR OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL CMAKE_SYSTEM_PROCESSOR)
            set(RUN_VCVARSALL_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}")
        else()
            set(RUN_VCVARSALL_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}_${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    endif()
    if(RUN_VCVARSALL_ARCH)
        list(APPEND RUN_VCVARSALL_ARGS "${RUN_VCVARSALL_ARCH}")
    endif()
    if(RUN_VCVARSALL_PLATFORM_TYPE)
        list(APPEND RUN_VCVARSALL_ARGS "${RUN_VCVARSALL_PLATFORM_TYPE}")
    endif()
    if(RUN_VCVARSALL_WINSDK_VERSION)
        list(APPEND RUN_VCVARSALL_ARGS "${RUN_VCVARSALL_WINSDK_VERSION}")
    endif()
    if(RUN_VCVARSALL_VC_VERSION)
        list(APPEND RUN_VCVARSALL_ARGS "-vcvars_ver=${RUN_VCVARSALL_VC_VERSION}")
    endif()
    if(RUN_VCVARSALL_SPECTRE_MODE)
        list(APPEND RUN_VCVARSALL_ARGS "-vcvars_spectre_libs=${RUN_VCVARSALL_SPECTRE_MODE}")
    endif()
    if(NOT RUN_VCVARSALL_ARGS)
        if(RUN_VCVARSALL_OPTIONAL)
            return()
        endif()
        message(FATAL_ERROR "Cannot run vcvarsall.bat with empty args")
    endif()
    list(JOIN RUN_VCVARSALL_ARGS " " RUN_VCVARSALL_ARGS_JOINED)
    message(DEBUG "Command: ${RUN_VCVARSALL_VCVARSALL_PATH}")
    message(DEBUG "Arguments: ${RUN_VCVARSALL_ARGS_JOINED}")
    set(RUN_VCVARSALL_COMMAND "${RUN_VCVARSALL_VCVARSALL_PATH}" ${RUN_VCVARSALL_ARGS})
    set(RUN_VCVARSALL_COMMAND cmd.exe /C ${RUN_VCVARSALL_COMMAND} && SET)
    execute_process(
        COMMAND ${RUN_VCVARSALL_COMMAND}
        OUTPUT_VARIABLE RUN_VCVARSALL_ENVS_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    string(REPLACE ";" "\\;" RUN_VCVARSALL_ENVS_OUTPUT "${RUN_VCVARSALL_ENVS_OUTPUT}")
    string(REGEX REPLACE "[\r\n]+" "|;" RUN_VCVARSALL_ENVS_OUTPUT "${RUN_VCVARSALL_ENVS_OUTPUT}")
    list(GET RUN_VCVARSALL_ENVS_OUTPUT 0 RUN_VCVARSALL_ENVS_OUTPUT_FIRST_LINE)
    string(FIND "${RUN_VCVARSALL_ENVS_OUTPUT_FIRST_LINE}" "[ERROR:vcvarsall.bat]" RUN_VCVARSALL_ENVS_OUTPUT_ERROR_LOC)
    if(RUN_VCVARSALL_ENVS_OUTPUT_ERROR_LOC GREATER -1)
        if(RUN_VCVARSALL_OPTIONAL)
            return()
        endif()
        message(FATAL_ERROR "Failed to run vcvarsall.bat")
    endif()
    foreach(LINE ${RUN_VCVARSALL_ENVS_OUTPUT})
        string(REGEX MATCH "^([^=]+)=(.*)[|]$" LINE_MATCH "${LINE}")
        if(LINE_MATCH)
            message(DEBUG "${CMAKE_MATCH_1}=${CMAKE_MATCH_2}")
            set(ENV{${CMAKE_MATCH_1}} "${CMAKE_MATCH_2}")
        endif()
    endforeach()
    message(DEBUG "PATH=$ENV{PATH}")
    message(DEBUG "INCLUDE=$ENV{INCLUDE}")
    message(DEBUG "LIB=$ENV{LIB}")
    message(DEBUG "VCINSTALLDIR=$ENV{VCINSTALLDIR}")
    message(DEBUG "WINDOWSSDKDIR=$ENV{WINDOWSSDKDIR}")
endfunction()

function(setup_msvc_vars)
    if(DEFINED ENV{VSCMD_VER})
        return()
    endif()
    set(OPTIONS
        OPTIONAL
    )
    set(ONE_VALUE_KEYWORDS)
    set(MULTI_VALUE_KEYWORDS)
    cmake_parse_arguments(SETUP_MSVC_VARS
        "${OPTIONS}"
        "${ONE_VALUE_KEYWORDS}"
        "${MULTI_VALUE_KEYWORDS}"
        ${ARGN}
    )
    run_vcvarsall(
        OPTIONAL SETUP_MSVC_VARS_OPTIONAL
    )
endfunction()

endblock()
