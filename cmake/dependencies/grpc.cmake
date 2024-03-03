include_guard(GLOBAL)

message(CHECK_START "Checking gRPC provider")

set(GRPC_CMAKE_CROSSCOMPILING "${CMAKE_CROSSCOMPILING}")
if(GRPC_CMAKE_CROSSCOMPILING
        AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME
        AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64"
        AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
    set(GRPC_CMAKE_CROSSCOMPILING FALSE)
endif()

if(AXSERVE_GRPC_PROVIDER STREQUAL "module")
    set(GRPC_EXTERNAL_NAME "gRPC")

    set(GRPC_SOURCE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/third_party/grpc")
    set(GRPC_PREFIX_DIR   "${CMAKE_CURRENT_BINARY_DIR}/grpc")
    set(GRPC_BINARY_DIR   "${CMAKE_CURRENT_BINARY_DIR}/grpc-build")
    set(GRPC_INSTALL_DIR  "${GRPC_PREFIX_DIR}")
    set(GRPC_TMP_DIR      "${GRPC_PREFIX_DIR}/tmp")
    set(GRPC_STAMP_DIR    "${GRPC_TMP_DIR}/stamp")
    set(GRPC_LOG_DIR      "${GRPC_TMP_DIR}/log")
    set(GRPC_DOWNLOAD_DIR "${GRPC_TMP_DIR}/download")

    if(NOT EXISTS "${GRPC_SOURCE_DIR}/CMakeLists.txt")
        set(GRPC_GIT_REPOSITORY "https://github.com/grpc/grpc.git")
        set(GRPC_GIT_TAG "v1.62.0")
    endif()

    set(GRPC_CMAKE_COMMAND "${CMAKE_COMMAND}")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(GRPC_CMAKE_ARGS
            "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
            "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
            "-P" "${CMAKE_CURRENT_LIST_DIR}/../msvc/env-msvc.cmake"
            "--" "${GRPC_CMAKE_COMMAND}")
        set(GRPC_BUILD_COMMAND
            "${GRPC_CMAKE_COMMAND}" ${GRPC_CMAKE_ARGS}
            "--build" "${GRPC_BINARY_DIR}"
            "--parallel")
        set(GRPC_INSTALL_COMMAND
            "${GRPC_CMAKE_COMMAND}" ${GRPC_CMAKE_ARGS}
            "--install" "${GRPC_BINARY_DIR}")
    endif()

    include("${CMAKE_CURRENT_LIST_DIR}/zlib.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/abseil-cpp.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/protobuf.cmake")
    set(GRPC_CMAKE_CACHE_ARGS
        "-DCMAKE_INSTALL_PREFIX:PATH=${GRPC_PREFIX_DIR}"
        "-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}"
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_CROSSCOMPILING:BOOL=${GRPC_CMAKE_CROSSCOMPILING}"
        "-DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DZLIB_USE_STATIC_LIBS:BOOL=${ZLIB_USE_STATIC_LIBS}"
        "-DgRPC_INSTALL:BOOL=ON"
        "-DgRPC_ABSL_PROVIDER:STRING=package"
        "-DgRPC_CARES_PROVIDER:STRING=module"
        "-DgRPC_PROTOBUF_PROVIDER:STRING=package"
        "-DgRPC_RE2_PROVIDER:STRING=module"
        "-DgRPC_SSL_PROVIDER:STRING=module"
        "-DgRPC_ZLIB_PROVIDER:STRING=package"
    )
    set(GRPC_DEPENDS "")
    if(AXSERVE_ZLIB_PROVIDER STREQUAL "module")
        list(APPEND GRPC_DEPENDS ZLIB)
    endif()
    if(AXSERVE_ABSL_PROVIDER STREQUAL "module")
        list(APPEND GRPC_DEPENDS absl)
    endif()
    if(AXSERVE_PROTOBUF_PROVIDER STREQUAL "module")
        list(APPEND GRPC_DEPENDS Protobuf)
    endif()

    include(ExternalProject)
    ExternalProject_Add("${GRPC_EXTERNAL_NAME}"
        PREFIX "${GRPC_PREFIX_DIR}"
        TMP_DIR "${GRPC_TMP_DIR}"
        STAMP_DIR "${GRPC_STAMP_DIR}"
        LOG_DIR "${GRPC_LOG_DIR}"
        DOWNLOAD_DIR "${GRPC_DOWNLOAD_DIR}"
        SOURCE_DIR "${GRPC_SOURCE_DIR}"
        BINARY_DIR "${GRPC_BINARY_DIR}"
        INSTALL_DIR "${GRPC_INSTALL_DIR}"
        GIT_REPOSITORY "${GRPC_GIT_REPOSITORY}"
        GIT_TAG "${GRPC_GIT_TAG}"
        GIT_SUBMODULES_RECURSE FALSE
        GIT_SHALLOW TRUE
        CMAKE_COMMAND "${GRPC_CMAKE_COMMAND}"
        CMAKE_ARGS ${GRPC_CMAKE_ARGS}
        CMAKE_CACHE_ARGS ${GRPC_CMAKE_CACHE_ARGS}
        BUILD_COMMAND ${GRPC_BUILD_COMMAND}
        INSTALL_COMMAND ${GRPC_INSTALL_COMMAND}
        DEPENDS ${GRPC_DEPENDS}
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
    )

    ExternalProject_Get_Property("${GRPC_EXTERNAL_NAME}" INSTALL_DIR)
    set(GRPC_INSTALL_DIR "${INSTALL_DIR}")
    list(APPEND CMAKE_PREFIX_PATH "${GRPC_INSTALL_DIR}")

    message(CHECK_PASS "${AXSERVE_GRPC_PROVIDER}")
elseif(AXSERVE_GRPC_PROVIDER STREQUAL "package")
    # x86 binaries can be run in amd64 host system
    # so temporarily turn off CMAKE_CROSSCOMPILING variable to import grpc plugin targets properly
    # required for accessing $<TARGET_FILE:gRPC::grpc_cpp_plugin> generator pattern in main CMakeLists.txt file
    if(CMAKE_CROSSCOMPILING)
        set(_CMAKE_CROSSCOMPILING_ORIG "${CMAKE_CROSSCOMPILING}")
        set(CMAKE_CROSSCOMPILING "${GRPC_CMAKE_CROSSCOMPILING}")
    endif()
    # find package and import targets
    find_package(gRPC CONFIG REQUIRED)
    # restore the original CMAKE_CROSSCOMPILING value
    if(DEFINED _CMAKE_CROSSCOMPILING_ORIG)
        set(CMAKE_CROSSCOMPILING "${_CMAKE_CROSSCOMPILING_ORIG}")
        unset(_CMAKE_CROSSCOMPILING_ORIG)
    endif()
    # done finding
    message(CHECK_PASS "${AXSERVE_GRPC_PROVIDER}")
else()
    message(CHECK_FAIL "${AXSERVE_GRPC_PROVIDER}")
endif()
