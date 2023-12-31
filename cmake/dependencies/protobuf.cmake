include_guard(GLOBAL)

message(CHECK_START "Checking Protobuf provider")

if (NOT BUILD_SHARED_LIBS)
    set(Protobuf_USE_STATIC_LIBS ON)
endif()

set(PROTOBUF_CMAKE_CROSSCOMPILING "${CMAKE_CROSSCOMPILING}")
if(PROTOBUF_CMAKE_CROSSCOMPILING
        AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME
        AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64"
        AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
    set(PROTOBUF_CMAKE_CROSSCOMPILING FALSE)
endif()

if(AXSERVE_PROTOBUF_PROVIDER STREQUAL "module")
    set(PROTOBUF_EXTERNAL_NAME "Protobuf")

    set(PROTOBUF_SOURCE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/third_party/protobuf")
    set(PROTOBUF_PREFIX_DIR   "${CMAKE_CURRENT_BINARY_DIR}/protobuf")
    set(PROTOBUF_BINARY_DIR   "${CMAKE_CURRENT_BINARY_DIR}/protobuf-build")
    set(PROTOBUF_INSTALL_DIR  "${PROTOBUF_PREFIX_DIR}")
    set(PROTOBUF_TMP_DIR      "${PROTOBUF_PREFIX_DIR}/tmp")
    set(PROTOBUF_STAMP_DIR    "${PROTOBUF_TMP_DIR}/stamp")
    set(PROTOBUF_LOG_DIR      "${PROTOBUF_TMP_DIR}/log")
    set(PROTOBUF_DOWNLOAD_DIR "${PROTOBUF_TMP_DIR}/download")

    if(NOT EXISTS "${PROTOBUF_SOURCE_DIR}/CMakeLists.txt")
        set(PROTOBUF_GIT_REPOSITORY "https://github.com/protocolbuffers/protobuf.git")
        set(PROTOBUF_GIT_TAG "v25.1")
    endif()

    set(PROTOBUF_CMAKE_COMMAND "${CMAKE_COMMAND}")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(PROTOBUF_CMAKE_ARGS
            "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
            "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
            "-P" "${CMAKE_CURRENT_LIST_DIR}/../msvc/env-msvc.cmake"
            "--" "${PROTOBUF_CMAKE_COMMAND}")
        set(PROTOBUF_BUILD_COMMAND
            "${PROTOBUF_CMAKE_COMMAND}" ${PROTOBUF_CMAKE_ARGS}
            "--build" "${PROTOBUF_BINARY_DIR}"
            "--parallel")
        set(PROTOBUF_INSTALL_COMMAND
            "${PROTOBUF_CMAKE_COMMAND}" ${PROTOBUF_CMAKE_ARGS}
            "--install" "${PROTOBUF_BINARY_DIR}")
        set(PROTOBUF_PATCH_COMMAND
            git restore * &&
            git apply --ignore-whitespace "${CMAKE_CURRENT_LIST_DIR}/../../patches/protobuf-v25.1.patch")
    endif()

    include("${CMAKE_CURRENT_LIST_DIR}/googletest.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/zlib.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/abseil-cpp.cmake")
    set(PROTOBUF_CMAKE_CACHE_ARGS
        "-DCMAKE_INSTALL_PREFIX:PATH=${PROTOBUF_PREFIX_DIR}"
        "-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}"
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_CROSSCOMPILING:BOOL=${PROTOBUF_CMAKE_CROSSCOMPILING}"
        "-DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DZLIB_USE_STATIC_LIBS:BOOL=${ZLIB_USE_STATIC_LIBS}"
        "-Dprotobuf_INSTALL:BOOl=ON"
        "-Dprotobuf_USE_EXTERNAL_GTEST:BOOL=ON"
        "-Dprotobuf_WITH_ZLIB:BOOL=ON"
        "-Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF"
        "-Dprotobuf_ABSL_PROVIDER:STRING=package"
    )
    set(PROTOBUF_DEPENDS "")
    if(AXSERVE_GTEST_PROVIDER STREQUAL "module")
        list(APPEND PROTOBUF_DEPENDS GTest)
    endif()
    if(AXSERVE_ZLIB_PROVIDER STREQUAL "module")
        list(APPEND PROTOBUF_DEPENDS ZLIB)
    endif()
    if(AXSERVE_ABSL_PROVIDER STREQUAL "module")
        list(APPEND PROTOBUF_DEPENDS absl)
    endif()

    include(ExternalProject)
    ExternalProject_Add("${PROTOBUF_EXTERNAL_NAME}"
        PREFIX "${PROTOBUF_PREFIX_DIR}"
        TMP_DIR "${PROTOBUF_TMP_DIR}"
        STAMP_DIR "${PROTOBUF_STAMP_DIR}"
        LOG_DIR "${PROTOBUF_LOG_DIR}"
        DOWNLOAD_DIR "${PROTOBUF_DOWNLOAD_DIR}"
        SOURCE_DIR "${PROTOBUF_SOURCE_DIR}"
        BINARY_DIR "${PROTOBUF_BINARY_DIR}"
        INSTALL_DIR "${PROTOBUF_INSTALL_DIR}"
        GIT_REPOSITORY "${PROTOBUF_GIT_REPOSITORY}"
        GIT_TAG "${PROTOBUF_GIT_TAG}"
        GIT_SUBMODULES_RECURSE FALSE
        GIT_SHALLOW TRUE
        CMAKE_COMMAND "${PROTOBUF_CMAKE_COMMAND}"
        CMAKE_ARGS ${PROTOBUF_CMAKE_ARGS}
        CMAKE_CACHE_ARGS ${PROTOBUF_CMAKE_CACHE_ARGS}
        BUILD_COMMAND ${PROTOBUF_BUILD_COMMAND}
        INSTALL_COMMAND ${PROTOBUF_INSTALL_COMMAND}
        PATCH_COMMAND  ${PROTOBUF_PATCH_COMMAND}
        DEPENDS ${PROTOBUF_DEPENDS}
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
    )

    ExternalProject_Get_Property("${PROTOBUF_EXTERNAL_NAME}" INSTALL_DIR)
    set(PROTOBUF_INSTALL_DIR "${INSTALL_DIR}")
    list(APPEND CMAKE_PREFIX_PATH "${PROTOBUF_INSTALL_DIR}")

    message(CHECK_PASS "${AXSERVE_PROTOBUF_PROVIDER}")
elseif(AXSERVE_PROTOBUF_PROVIDER STREQUAL "package")
    find_package(Protobuf CONFIG REQUIRED)
    message(CHECK_PASS "${AXSERVE_PROTOBUF_PROVIDER}")
else()
    message(CHECK_FAIL "${AXSERVE_PROTOBUF_PROVIDER}")
endif()
