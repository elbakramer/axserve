cmake_minimum_required(VERSION 3.27)

include_guard(GLOBAL)

message(CHECK_START "Checking GoogleTest provider")

if (BUILD_SHARED_LIBS)
    set(GTEST_CREATE_SHARED_LIBRARY 1)
    set(GTEST_LINKED_AS_SHARED_LIBRARY 1)
else()
    set(GTEST_CREATE_SHARED_LIBRARY 0)
    set(GTEST_LINKED_AS_SHARED_LIBRARY 0)
endif()

if(AXSERVE_GTEST_PROVIDER STREQUAL "module")
    set(GTEST_EXTERNAL_NAME "GTest")
    set(GTEST_PREFIX_NAME "gtest")
    set(GTEST_THIRD_PARTY_NAME "googletest")

    set(GTEST_GIT_REPOSITORY "https://github.com/google/googletest.git")
    set(GTEST_GIT_TAG "v1.17.0")

    set(GTEST_CMAKE_CACHE_ARGS
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_VERSION:STRING=${CMAKE_SYSTEM_VERSION}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
        "-Dgtest_build_tests:BOOL=${BUILD_TESTING}"
        "-Dgtest_build_samples:BOOL=${BUILD_SAMPLES}"
        "-Dgtest_force_shared_crt:BOOL=ON"
        "-DBUILD_GMOCK:BOOL=ON"
    )

    include("${CMAKE_CURRENT_LIST_DIR}/../util/external-project.cmake")

    ExternalProject_AddForThisProject("${GTEST_EXTERNAL_NAME}"
        PREFIX_NAME "${GTEST_PREFIX_NAME}"
        THIRD_PARTY_NAME "${GTEST_THIRD_PARTY_NAME}"
        GIT_REPOSITORY "${GTEST_GIT_REPOSITORY}"
        GIT_TAG "${GTEST_GIT_TAG}"
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
        LOG_OUTPUT_ON_FAILURE TRUE
        CMAKE_CACHE_ARGS ${GTEST_CMAKE_CACHE_ARGS}
        DEPENDS ${GTEST_DEPENDS}
    )

    message(CHECK_PASS "${AXSERVE_GTEST_PROVIDER}")
elseif(AXSERVE_GTEST_PROVIDER STREQUAL "package")
    find_package(GTest REQUIRED)
    message(CHECK_PASS "${AXSERVE_GTEST_PROVIDER}")
else()
    message(CHECK_FAIL "${AXSERVE_GTEST_PROVIDER}")
endif()
