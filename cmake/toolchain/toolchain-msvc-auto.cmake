# always build for windows system target
set(CMAKE_SYSTEM_NAME Windows)

# start toolchain message
message(CHECK_START "Configure toolchain")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

# check host system information
cmake_host_system_information(RESULT CMAKE_HOST_SYSTEM_NAME QUERY OS_NAME)
cmake_host_system_information(RESULT CMAKE_HOST_SYSTEM_PROCESSOR QUERY OS_PLATFORM)

# use check_language here
include(CheckLanguage)

# extend check_language macro to
# - ignore predefined CMAKE_${lang}_COMPILER variable if it's invalid like NOTFOUND
# - ignore predefined CMAKE_TOOLCHAIN_FILE variable and restore later (to prevent including toolchain file recursively)
macro(_clean_check_language lang)
    if(NOT CMAKE_${lang}_COMPILER)
        unset(CMAKE_${lang}_COMPILER)
        unset(CMAKE_${lang}_COMPILER CACHE)
    endif()
    if(DEFINED CMAKE_TOOLCHAIN_FILE)
        set(_CMAKE_TOOLCHAIN_FILE_ORIG "${CMAKE_TOOLCHAIN_FILE}")
        unset(CMAKE_TOOLCHAIN_FILE)
    endif()
    if(DEFINED CACHE{CMAKE_TOOLCHAIN_FILE})
        set(_CMAKE_TOOLCHAIN_FILE_CACHE_ORIG "${CMAKE_TOOLCHAIN_FILE}")
        get_property(_CMAKE_TOOLCHAIN_FILE_CACHE_ORIG_DOCSTRING CACHE CMAKE_TOOLCHAIN_FILE PROPERTY DOCSTRING)
        unset(CMAKE_TOOLCHAIN_FILE CACHE)
    endif()
    check_language(${lang})
    if(DEFINED _CMAKE_TOOLCHAIN_FILE_CACHE_ORIG)
        set(CMAKE_TOOLCHAIN_FILE "${_CMAKE_TOOLCHAIN_FILE_CACHE_ORIG}" CACHE PATH "${_CMAKE_TOOLCHAIN_FILE_CACHE_ORIG_DOCSTRING}")
        unset(_CMAKE_TOOLCHAIN_FILE_CACHE_ORIG)
        unset(_CMAKE_TOOLCHAIN_FILE_CACHE_ORIG_DOCSTRING)
    endif()
    if(DEFINED _CMAKE_TOOLCHAIN_FILE_ORIG)
        set(CMAKE_TOOLCHAIN_FILE "${_CMAKE_TOOLCHAIN_FILE_ORIG}")
        unset(_CMAKE_TOOLCHAIN_FILE_ORIG)
    endif()
endmacro()

# check CMAKE_GENERATOR and give default value to CMAKE_MAKE_PROGRAM before checking language
if(CMAKE_GENERATOR AND NOT CMAKE_MAKE_PROGRAM)
    include(CMake${CMAKE_GENERATOR}FindMake OPTIONAL)
endif()

# check cxx compiler
message(CHECK_START "Looking for a CXX compiler in PATH")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
_clean_check_language(CXX)
list(POP_BACK CMAKE_MESSAGE_INDENT)
if(CMAKE_CXX_COMPILER)
    message(CHECK_PASS "${CMAKE_CXX_COMPILER}")
else()
    message(CHECK_FAIL "not found")
endif()

# if no cxx compiler found and also no system processor given, infer system processor to follow the host's
# the inferred system processor will be used later in setting up the msvc cxx compiler
if(NOT CMAKE_CXX_COMPILER AND NOT CMAKE_SYSTEM_PROCESSOR AND CMAKE_HOST_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR "${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

# if no cxx compiler found and system processor is given and we are building in windows host
# setup msvc env vars for checking cxx compiler again
if(NOT CMAKE_CXX_COMPILER AND CMAKE_SYSTEM_PROCESSOR AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    message(CHECK_START "Looking for a CXX compiler after loading MSVC environment variables")
    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    if(NOT VS_SETUP)
        message(CHECK_START "Finding Visual Studio BuildTools")
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include("${CMAKE_CURRENT_LIST_DIR}/../msvc/setup-msvc-vars.cmake")
        list(POP_BACK CMAKE_MESSAGE_INDENT)
        if(VS_SETUP)
            if(VS_INSTALLATION_PATH)
                message(CHECK_PASS "${VS_INSTALLATION_PATH}")
            else()
                message(CHECK_PASS "found")
            endif()
        else()
            message(CHECK_FAIL "not found")
        endif()
    endif()
    if(VS_SETUP)
        _clean_check_language(CXX)
    endif()
    list(POP_BACK CMAKE_MESSAGE_INDENT)
    if(CMAKE_CXX_COMPILER)
        message(CHECK_PASS "${CMAKE_CXX_COMPILER}")
    else()
        message(CHECK_FAIL "not found")
    endif()
endif()

# at this point, we cannot further infer the cxx compiler, so abort
if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "Failed to determine CXX compiler")
endif()

# we can determine the cxx compiler, determine compiler here
message(CHECK_START "Checking CXX compiler identification")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
include(CMakeDetermineCXXCompiler)
list(POP_BACK CMAKE_MESSAGE_INDENT)
if(CMAKE_CXX_COMPILER_ID)
    include("${CMAKE_PLATFORM_INFO_DIR}/CMakeCXXCompiler.cmake" OPTIONAL)
    message(CHECK_PASS "${CMAKE_CXX_COMPILER_ID}")
else()
    message(CHECK_FAIL "failed")
endif()

# if we don't have the system processor information yet, infer from the determined cxx compiler
if(NOT CMAKE_SYSTEM_PROCESSOR AND CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
    if (CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "x64")
        set(CMAKE_SYSTEM_PROCESSOR "AMD64")
    elseif (CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "X86")
        set(CMAKE_SYSTEM_PROCESSOR "x86")
    else()
        set(CMAKE_SYSTEM_PROCESSOR "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}")
    endif()
endif()

# at this point, we cannot further infer the system processor, so abort
if(NOT CMAKE_SYSTEM_PROCESSOR)
    message(FATAL_ERROR "Failed to determine system processor")
endif()

# if no cross-compiling flag is given, infer based on system information
if(NOT DEFINED CMAKE_CROSSCOMPILING)
    if(CMAKE_SYSTEM_NAME AND CMAKE_HOST_SYSTEM_NAME AND NOT CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
        set(CMAKE_CROSSCOMPILING TRUE)
    elseif(CMAKE_SYSTEM_PROCESSOR AND CMAKE_HOST_SYSTEM_PROCESSOR AND NOT CMAKE_SYSTEM_PROCESSOR STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR)
        set(CMAKE_CROSSCOMPILING TRUE)
    else()
        set(CMAKE_CROSSCOMPILING FALSE)
    endif()
endif()

# if we are using msvc, setup environment variables
# before configuring further or doing other things
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT VS_SETUP)
    include("${CMAKE_CURRENT_LIST_DIR}/../msvc/setup-msvc-vars.cmake")
endif()

# end toolchain message
list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "done")
