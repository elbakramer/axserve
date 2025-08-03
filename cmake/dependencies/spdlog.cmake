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

message(CHECK_START "Checking spdlog provider")

if(AXSERVE_SPDLOG_PROVIDER STREQUAL "module")
    set(SPDLOG_EXTERNAL_NAME "spdlog")
    set(SPDLOG_PREFIX_NAME "spdlog")
    set(SPDLOG_THIRD_PARTY_NAME "spdlog")

    set(SPDLOG_GIT_REPOSITORY "https://github.com/gabime/spdlog.git")
    set(SPDLOG_GIT_TAG "v1.15.3")

    set(SPDLOG_CMAKE_CACHE_ARGS
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_VERSION:STRING=${CMAKE_SYSTEM_VERSION}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
    )

    include("${CMAKE_CURRENT_LIST_DIR}/../util/external-project.cmake")

    ExternalProject_AddForThisProject("${SPDLOG_EXTERNAL_NAME}"
        PREFIX_NAME "${SPDLOG_PREFIX_NAME}"
        THIRD_PARTY_NAME "${SPDLOG_THIRD_PARTY_NAME}"
        GIT_REPOSITORY "${SPDLOG_GIT_REPOSITORY}"
        GIT_TAG "${SPDLOG_GIT_TAG}"
        CMAKE_CACHE_ARGS ${SPDLOG_CMAKE_CACHE_ARGS}
        DEPENDS ${SPDLOG_DEPENDS}
        START_UNPARSED_ARGUMENTS
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
        LOG_OUTPUT_ON_FAILURE TRUE
    )

    message(CHECK_PASS "${AXSERVE_SPDLOG_PROVIDER}")
elseif(AXSERVE_SPDLOG_PROVIDER STREQUAL "package")
    find_package(spdlog CONFIG REQUIRED)
    message(CHECK_PASS "${AXSERVE_SPDLOG_PROVIDER}")
else()
    message(CHECK_FAIL "${AXSERVE_SPDLOG_PROVIDER}")
endif()
