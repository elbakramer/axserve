# Copyright 2025 Yunseong Hwang
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-FileCopyrightText: 2025 Yunseong Hwang
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.27)

include_guard(GLOBAL)

message(CHECK_START "Checking Protobuf provider")

if (BUILD_SHARED_LIBS)
    set(Protobuf_USE_STATIC_LIBS OFF)
else()
    set(Protobuf_USE_STATIC_LIBS ON)
endif()

if(AXSERVE_PROTOBUF_PROVIDER STREQUAL "module")
    set(PROTOBUF_EXTERNAL_NAME "Protobuf")
    set(PROTOBUF_PREFIX_NAME "protobuf")
    set(PROTOBUF_THIRD_PARTY_NAME "protobuf")

    set(PROTOBUF_GIT_REPOSITORY "https://github.com/protocolbuffers/protobuf.git")
    set(PROTOBUF_GIT_TAG "v31.1")

    include("${CMAKE_CURRENT_LIST_DIR}/googletest.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/zlib.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/abseil-cpp.cmake")

    if (BUILD_SHARED_LIBS)
        set(ZLIB_USE_STATIC_LIBS OFF)
    else()
        set(ZLIB_USE_STATIC_LIBS ON)
    endif()

    string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" CMAKE_HOST_SYSTEM_PROCESSOR_LOWER)
    list(APPEND CMAKE_PROGRAM_PATH "${CMAKE_CURRENT_BINARY_DIR}/../${CMAKE_HOST_SYSTEM_PROCESSOR_LOWER}/${PROTOBUF_PREFIX_NAME}-release/bin")

    set(PROTOBUF_CMAKE_CACHE_ARGS
        "-DCMAKE_PROGRAM_PATH:PATH=${CMAKE_PROGRAM_PATH}"
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_VERSION:STRING=${CMAKE_SYSTEM_VERSION}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
        "-Dprotobuf_BUILD_TESTS:BOOL=${BUILD_TESTING}"
        "-Dprotobuf_INSTALL:BOOL=ON"
        "-Dprotobuf_USE_EXTERNAL_GTEST:BOOL=ON"
        "-Dprotobuf_WITH_ZLIB:BOOL=ON"
        "-Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF"
        "-Dprotobuf_ABSL_PROVIDER:STRING=package"
        "-Dprotobuf_JSONCPP_PROVIDER :STRING=module"
        "-DZLIB_USE_STATIC_LIBS:BOOL=${ZLIB_USE_STATIC_LIBS}"
    )

    if(AXSERVE_GTEST_PROVIDER STREQUAL "module")
        list(APPEND PROTOBUF_DEPENDS GTest)
    endif()
    if(AXSERVE_ZLIB_PROVIDER STREQUAL "module")
        list(APPEND PROTOBUF_DEPENDS ZLIB)
    endif()
    if(AXSERVE_ABSL_PROVIDER STREQUAL "module")
        list(APPEND PROTOBUF_DEPENDS absl)
    endif()

    include("${CMAKE_CURRENT_LIST_DIR}/../util/external-project.cmake")

    ExternalProject_AddForThisProject("${PROTOBUF_EXTERNAL_NAME}"
        PREFIX_NAME "${PROTOBUF_PREFIX_NAME}"
        THIRD_PARTY_NAME "${PROTOBUF_THIRD_PARTY_NAME}"
        GIT_REPOSITORY "${PROTOBUF_GIT_REPOSITORY}"
        GIT_TAG "${PROTOBUF_GIT_TAG}"
        GIT_SUBMODULES_RECURSE FALSE
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        LOG_DOWNLOAD TRUE
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
        CMAKE_ARGS ${PROTOBUF_CMAKE_ARGS}
        CMAKE_CACHE_ARGS ${PROTOBUF_CMAKE_CACHE_ARGS}
        DEPENDS ${PROTOBUF_DEPENDS}
    )

    message(CHECK_PASS "${AXSERVE_PROTOBUF_PROVIDER}")
elseif(AXSERVE_PROTOBUF_PROVIDER STREQUAL "package")
    find_package(Protobuf CONFIG REQUIRED)
    message(CHECK_PASS "${AXSERVE_PROTOBUF_PROVIDER}")
else()
    message(CHECK_FAIL "${AXSERVE_PROTOBUF_PROVIDER}")
endif()
