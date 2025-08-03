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

message(CHECK_START "Checking Qt6 provider")

if(NOT CMAKE_GENERATOR MATCHES "Ninja")
    message(FATAL_ERROR "Qt6 supports Ninja generators only")
endif()

set(QT6_COMPONENTS Widgets AxContainer Concurrent Network LinguistTools)
set(QT6_SUBMODULES qtbase qtactiveqt qttools qtrepotools)

if(AXSERVE_QT6_PROVIDER STREQUAL "module")
    set(QT6_EXTERNAL_NAME "Qt6")
    set(QT6_PREFIX_NAME "qt")
    set(QT6_THIRD_PARTY_NAME "qt")

    set(QT6_GIT_REPOSITORY "https://code.qt.io/qt/qt5.git")
    set(QT6_GIT_TAG "v6.9.1")
    set(QT6_GIT_SUBMODULES ${QT6_SUBMODULES})

    include("${CMAKE_CURRENT_LIST_DIR}/zlib.cmake")

    if (BUILD_SHARED_LIBS)
        set(ZLIB_USE_STATIC_LIBS OFF)
    else()
        set(ZLIB_USE_STATIC_LIBS ON)
    endif()

    string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" CMAKE_HOST_SYSTEM_PROCESSOR_LOWER)
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/../${CMAKE_HOST_SYSTEM_PROCESSOR_LOWER}/${QT6_PREFIX_NAME}-release")
    list(APPEND CMAKE_PROGRAM_PATH "${CMAKE_CURRENT_BINARY_DIR}/../${CMAKE_HOST_SYSTEM_PROCESSOR_LOWER}/${QT6_PREFIX_NAME}-release/bin")

    set(QT6_CMAKE_CACHE_ARGS
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_VERSION:STRING=${CMAKE_SYSTEM_VERSION}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
        "-DQT_BUILD_TESTS:BOOL=${BUILD_TESTING}"
        "-DQT_BUILD_TESTS:BOOL=${BUILD_TESTING}"
        "-DQT_BUILD_EXAMPLES:BOOL=${BUILD_SAMPLES}"
        "-DBUILD_qtwebengine:BOOL=OFF"
        "-DFEATURE_system_zlib:BOOL=ON"
        "-DZLIB_USE_STATIC_LIBS:BOOL=${ZLIB_USE_STATIC_LIBS}"
    )

    if(CMAKE_CXX_STANDARD EQUAL 20)
        list(APPEND QT6_CMAKE_CACHE_ARGS "-DFEATURE_cxx20:BOOL=ON")
    endif()

    if(CMAKE_CROSSCOMPILING)
        string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" CMAKE_HOST_SYSTEM_PROCESSOR_LOWER)
        set(QT_HOST_PATH "${CMAKE_CURRENT_BINARY_DIR}/../${CMAKE_HOST_SYSTEM_PROCESSOR_LOWER}/${QT6_PREFIX_NAME}-release")
        list(APPEND QT6_CMAKE_CACHE_ARGS "-DQT_HOST_PATH:PATH=${QT_HOST_PATH}")
    endif()

    if(AXSERVE_ZLIB_PROVIDER STREQUAL "module")
        list(APPEND QT6_DEPENDS ZLIB)
    endif()

    include("${CMAKE_CURRENT_LIST_DIR}/../util/external-project.cmake")

    ExternalProject_AddForThisProject("${QT6_EXTERNAL_NAME}"
        PREFIX_NAME "${QT6_PREFIX_NAME}"
        THIRD_PARTY_NAME "${QT6_THIRD_PARTY_NAME}"
        GIT_REPOSITORY "${QT6_GIT_REPOSITORY}"
        GIT_TAG "${QT6_GIT_TAG}"
        GIT_SUBMODULES ${QT6_GIT_SUBMODULES}
        GIT_SUBMODULES_RECURSE TRUE
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        LOG_DOWNLOAD TRUE
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
        LOG_OUTPUT_ON_FAILURE TRUE
        CMAKE_ARGS ${QT6_CMAKE_ARGS}
        CMAKE_CACHE_ARGS ${QT6_CMAKE_CACHE_ARGS}
        DEPENDS ${QT6_DEPENDS}
        # SKIP_BUILD TRUE
    )

    message(CHECK_PASS "${AXSERVE_QT6_PROVIDER}")
elseif(AXSERVE_QT6_PROVIDER STREQUAL "package")
    find_package(Qt6 REQUIRED COMPONENTS ${QT6_COMPONENTS})
    get_target_property(QT6_AUTOMOC_EXECUTABLE Qt6::moc IMPORTED_LOCATION)
    get_target_property(QT6_AUTOUIC_EXECUTABLE Qt6::uic IMPORTED_LOCATION)
    get_target_property(QT6_AUTORCC_EXECUTABLE Qt6::rcc IMPORTED_LOCATION)
    if(NOT QT6_AUTOMOC_EXECUTABLE)
        find_program(QT6_AUTOMOC_EXECUTABLE REQUIRED NAMES moc)
        set_target_properties(Qt6::moc PROPERTIES IMPORTED_LOCATION "${QT6_AUTOMOC_EXECUTABLE}")
    endif()
    if(NOT QT6_AUTOUIC_EXECUTABLE)
        find_program(QT6_AUTOUIC_EXECUTABLE REQUIRED NAMES uic)
        set_target_properties(Qt6::uic PROPERTIES IMPORTED_LOCATION "${QT6_AUTOUIC_EXECUTABLE}")
    endif()
    if(NOT QT6_AUTORCC_EXECUTABLE)
        find_program(QT6_AUTORCC_EXECUTABLE REQUIRED NAMES rcc)
        set_target_properties(Qt6::rcc PROPERTIES IMPORTED_LOCATION "${QT6_AUTORCC_EXECUTABLE}")
    endif()
    get_target_property(QT6_LUPDATE_EXECUTABLE Qt6::lupdate IMPORTED_LOCATION)
    get_target_property(QT6_LRELEASE_EXECUTABLE Qt6::lrelease IMPORTED_LOCATION)
    get_target_property(QT6_WINDEPLOYQT_EXECUTABLE Qt6::windeployqt IMPORTED_LOCATION)
    if(NOT QT6_LUPDATE_EXECUTABLE)
        find_program(QT6_LUPDATE_EXECUTABLE REQUIRED NAMES lupdate)
        set_target_properties(Qt6::lupdate PROPERTIES IMPORTED_LOCATION "${QT6_LUPDATE_EXECUTABLE}")
    endif()
    if(NOT QT6_LRELEASE_EXECUTABLE)
        find_program(QT6_LRELEASE_EXECUTABLE REQUIRED NAMES lrelease)
        set_target_properties(Qt6::lrelease PROPERTIES IMPORTED_LOCATION "${QT6_LRELEASE_EXECUTABLE}")
    endif()
    if(NOT QT6_WINDEPLOYQT_EXECUTABLE)
        find_program(QT6_WINDEPLOYQT_EXECUTABLE REQUIRED NAMES windeployqt)
        set_target_properties(Qt6::windeployqt PROPERTIES IMPORTED_LOCATION "${QT6_WINDEPLOYQT_EXECUTABLE}")
    endif()
    message(CHECK_PASS "${AXSERVE_QT6_PROVIDER}")
else()
    message(CHECK_FAIL "${AXSERVE_QT6_PROVIDER}")
endif()
