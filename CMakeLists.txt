cmake_minimum_required(VERSION 3.27)

set(CMAKE_SYSTEM_NAME Windows CACHE STRING "")
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "")

project(AxServe VERSION 0.3.0 LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTING "Build tests" OFF)
option(BUILD_SAMPLES "Build samples" OFF)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "")

set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "")
set(CMAKE_INSTALL_LIBDIR "lib" CACHE STRING "")

# file and directory listings
file(GLOB_RECURSE PROJECT_CXX_FILES
    "${CMAKE_CURRENT_LIST_DIR}/src/cpp/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/cpp/*.cc"
    "${CMAKE_CURRENT_LIST_DIR}/src/cpp/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/cpp/*.h.in"
)
file(GLOB_RECURSE PROJECT_PROTO_FILES
    "${CMAKE_CURRENT_LIST_DIR}/src/proto/*.proto"
)
file(GLOB_RECURSE PROJECT_TS_FILES
    "${CMAKE_CURRENT_LIST_DIR}/translations/*.ts"
)

set(PROJECT_SOURCE_FILES ${PROJECT_CXX_FILES} ${PROJECT_PROTO_FILES} ${PROJECT_TS_FILES})

set(PROJECT_PROTO_IMPORT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/src/proto"
)

# cache variables for dependency resolution
set(AXSERVE_GTEST_PROVIDER "module" CACHE STRING "Provider of GoogleTest library")
set_property(CACHE AXSERVE_GTEST_PROVIDER PROPERTY STRINGS "module" "package")
set(AXSERVE_ZLIB_PROVIDER "module" CACHE STRING "Provider of zlib library")
set_property(CACHE AXSERVE_ZLIB_PROVIDER PROPERTY STRINGS "module" "package")
set(AXSERVE_ABSL_PROVIDER "module" CACHE STRING "Provider of Abseil C++ library")
set_property(CACHE AXSERVE_ABSL_PROVIDER PROPERTY STRINGS "module" "package")
set(AXSERVE_PROTOBUF_PROVIDER "module" CACHE STRING "Provider of Protobuf library")
set_property(CACHE AXSERVE_PROTOBUF_PROVIDER PROPERTY STRINGS "module" "package")
set(AXSERVE_GRPC_PROVIDER "module" CACHE STRING "Provider of gRPC library")
set_property(CACHE AXSERVE_GRPC_PROVIDER PROPERTY STRINGS "module" "package")
set(AXSERVE_QT6_PROVIDER "module" CACHE STRING "Provider of Qt6 library")
set_property(CACHE AXSERVE_QT6_PROVIDER PROPERTY STRINGS "module" "package")
set(AXSERVE_SPDLOG_PROVIDER "module" CACHE STRING "Provider of spdlog library")
set_property(CACHE AXSERVE_SPDLOG_PROVIDER PROPERTY STRINGS "module" "package")
set(AXSERVE_CLI11_PROVIDER "module" CACHE STRING "Provider of spdlog library")
set_property(CACHE AXSERVE_CLI11_PROVIDER PROPERTY STRINGS "module" "package")

# configure dependent packages
message(CHECK_START "Configure dependencies")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/googletest.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/zlib.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/abseil-cpp.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/protobuf.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/grpc.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/qt.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/spdlog.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/cli11.cmake")
list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "done")

# check dependency resolutions
if(AXSERVE_PROTOBUF_PROVIDER STREQUAL "module")
    list(APPEND AXSERVE_DEPENDS Protobuf)
endif()
if(AXSERVE_GRPC_PROVIDER STREQUAL "module")
    list(APPEND AXSERVE_DEPENDS gRPC)
endif()
if(AXSERVE_QT6_PROVIDER STREQUAL "module")
    list(APPEND AXSERVE_DEPENDS Qt6)
endif()
if(AXSERVE_SPDLOG_PROVIDER STREQUAL "module")
    list(APPEND AXSERVE_DEPENDS spdlog)
endif()
if(AXSERVE_CLI11_PROVIDER STREQUAL "module")
    list(APPEND AXSERVE_DEPENDS CLI11)
endif()

# configure executable names
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CMAKE_SYSTEM_PROCESSOR_LOWER)

set(AXSERVE_OUTPUT_NAME_WIN32 "${PROJECT_NAME_LOWER}-win32-${CMAKE_SYSTEM_PROCESSOR_LOWER}")
set(AXSERVE_OUTPUT_NAME_CONSOLE "${PROJECT_NAME_LOWER}-console-${CMAKE_SYSTEM_PROCESSOR_LOWER}")

# if necessary, build with external projects
if(AXSERVE_DEPENDS)

    # print status for super build
    string(REPLACE ";" ", " AXSERVE_DEPENDS_PRINTABLE "${AXSERVE_DEPENDS}")
    message(STATUS "Configure to build ${PROJECT_NAME} with other deps including ${AXSERVE_DEPENDS_PRINTABLE} using ExternalProject")

    # cmake args to pass
    set(AXSERVE_CMAKE_CACHE_ARGS
        "-DCMAKE_PROGRAM_PATH:PATH=${CMAKE_PROGRAM_PATH}"
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_VERSION:STRING=${CMAKE_SYSTEM_VERSION}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
        "-DBUILD_SAMPLES:BOOL=${BUILD_SAMPLES}"
        "-DAXSERVE_GTEST_PROVIDER:STRING=package"
        "-DAXSERVE_ZLIB_PROVIDER:STRING=package"
        "-DAXSERVE_ABSL_PROVIDER:STRING=package"
        "-DAXSERVE_PROTOBUF_PROVIDER:STRING=package"
        "-DAXSERVE_GRPC_PROVIDER:STRING=package"
        "-DAXSERVE_QT6_PROVIDER:STRING=package"
        "-DAXSERVE_SPDLOG_PROVIDER:STRING=package"
        "-DAXSERVE_CLI11_PROVIDER:STRING=package"
        "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )

    # import external project library
    include("${CMAKE_CURRENT_LIST_DIR}/cmake/util/external-project.cmake")

    # external project for axserve inside super build
    ExternalProject_AddForThisProject("${PROJECT_NAME}"
        PREFIX_NAME "${PROJECT_NAME_LOWER}"
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
        LOG_OUTPUT_ON_FAILURE TRUE
        BUILD_ALWAYS TRUE
        CMAKE_ARGS ${AXSERVE_CMAKE_ARGS}
        CMAKE_CACHE_ARGS ${AXSERVE_CMAKE_CACHE_ARGS}
        DEPENDS ${AXSERVE_DEPENDS}
    )

    # add steps related to translations
    ExternalProject_Add_Step("${PROJECT_NAME}" update_translations
        COMMAND cmake --build <BINARY_DIR> --target update_translations $<$<BOOL:$<CONFIG>>:--config> $<CONFIG>
        WORKING_DIRECTORY <SOURCE_DIR>
        ALWAYS TRUE
        EXCLUDE_FROM_MAIN TRUE
    )
    ExternalProject_Add_StepTargets("${PROJECT_NAME}" update_translations)
    ExternalProject_Add_Step("${PROJECT_NAME}" release_translations
        COMMAND cmake --build <BINARY_DIR> --target release_translations $<$<BOOL:$<CONFIG>>:--config> $<CONFIG>
        WORKING_DIRECTORY <SOURCE_DIR>
        ALWAYS TRUE
        EXCLUDE_FROM_MAIN TRUE
    )
    ExternalProject_Add_StepTargets("${PROJECT_NAME}" release_translations)

    # get the runtimes from the external project
    get_property(INSTALL_DIR TARGET "${PROJECT_NAME}" PROPERTY PREFIX_PATH)

    add_executable(axserve_executable_win32 IMPORTED)
    set_target_properties(axserve_executable_win32 PROPERTIES
        IMPORTED_LOCATION "${INSTALL_DIR}/bin/${AXSERVE_OUTPUT_NAME_WIN32}.exe"
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_WIN32}"
    )
    add_executable(axserve_executable_console IMPORTED)
    set_target_properties(axserve_executable_console PROPERTIES
        IMPORTED_LOCATION "${INSTALL_DIR}/bin/${AXSERVE_OUTPUT_NAME_CONSOLE}.exe"
        WIN32_EXECUTABLE FALSE
        OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_CONSOLE}"
    )

    set(AXSERVE_INSTALL_IMPORTED_RUNTIME_ARTIFACTS
        axserve_executable_win32
        axserve_executable_console
    )

    # configure install dirs
    install(IMPORTED_RUNTIME_ARTIFACTS ${AXSERVE_INSTALL_IMPORTED_RUNTIME_ARTIFACTS}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

# else proceed with normal cmake lists
else()

    # print status for normal build
    message(STATUS "Configure to build ${PROJECT_NAME} as normal, without external projects")

    # set organization information for qsettings
    set(AXSERVE_ORG_NAME "Yunseong")
    set(AXSERVE_ORG_DOMAIN "yunseongh.wang")

    # set application information
    set(AXSERVE_APP_DISPLAY_NAME "${PROJECT_NAME}")
    set(AXSERVE_APP_VERSION "${PROJECT_VERSION}")

    # enable auto features for qt related files
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    # configure and generated output locations
    set(AXSERVE_CONFIGURE_OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/generated/cpp")
    set(AXSERVE_PROTO_OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/generated/proto")
    set(AXSERVE_TRANSLATION_OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")

    # configure headers
    set(AXSERVE_APP_NAME "${AXSERVE_OUTPUT_NAME_WIN32}")
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/src/cpp/axserve/app/config.h.in"
        "${AXSERVE_CONFIGURE_OUTPUT_LOCATION}/win32/axserve/app/config.h"
    )
    set(AXSERVE_APP_NAME "${AXSERVE_OUTPUT_NAME_CONSOLE}")
    configure_file(
        "${CMAKE_CURRENT_LIST_DIR}/src/cpp/axserve/app/config.h.in"
        "${AXSERVE_CONFIGURE_OUTPUT_LOCATION}/console/axserve/app/config.h"
    )

    # add common proto library with protobuf generation
    add_library(axserve_proto ${PROJECT_PROTO_FILES})
    target_link_libraries(axserve_proto PUBLIC
        gRPC::grpc++
        gRPC::grpc++_reflection
    )
    target_link_libraries(axserve_proto PUBLIC
        protobuf::libprotobuf
    )
    target_include_directories(axserve_proto PUBLIC
        "$<BUILD_INTERFACE:${AXSERVE_PROTO_OUTPUT_LOCATION}>"
    )

    protobuf_generate(
        TARGET axserve_proto
        LANGUAGE cpp
        IMPORT_DIRS ${PROJECT_PROTO_IMPORT_DIRS}
        PROTOC_OUT_DIR "${AXSERVE_PROTO_OUTPUT_LOCATION}"
    )
    protobuf_generate(
        TARGET axserve_proto
        LANGUAGE grpc
        IMPORT_DIRS ${PROJECT_PROTO_IMPORT_DIRS}
        PROTOC_OUT_DIR "${AXSERVE_PROTO_OUTPUT_LOCATION}"
        PLUGIN "protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}"
        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    )

    # set translation files output location
    set_source_files_properties(${PROJECT_TS_FILES} PROPERTIES
        OUTPUT_LOCATION "${AXSERVE_TRANSLATION_OUTPUT_LOCATION}"
    )

    # setup common library for executables
    add_library(axserve_common)
    target_link_libraries(axserve_common PUBLIC
        Qt6::Widgets
        Qt6::Network
        Qt6::AxContainer
        Qt6::Concurrent
        CLI11::CLI11
        spdlog::spdlog
    )
    target_link_libraries(axserve_common PUBLIC
        axserve_proto
    )

    # setup win32 executable
    add_library(axserve_common_win32 ${PROJECT_SOURCE_FILES})
    target_link_libraries(axserve_common_win32 PUBLIC
        axserve_common
    )
    target_include_directories(axserve_common_win32 PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/src/cpp"
        "$<BUILD_INTERFACE:${AXSERVE_CONFIGURE_OUTPUT_LOCATION}>/win32"
    )
    qt_add_translations(axserve_common_win32
        TS_FILES ${PROJECT_TS_FILES}
        NO_GENERATE_PLURALS_TS_FILE
    )

    # setup console executable
    add_library(axserve_common_console ${PROJECT_SOURCE_FILES})
    target_link_libraries(axserve_common_console PUBLIC
        axserve_common
    )
    target_include_directories(axserve_common_console PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/src/cpp"
        "$<BUILD_INTERFACE:${AXSERVE_CONFIGURE_OUTPUT_LOCATION}>/console"
    )
    qt_add_translations(axserve_common_console
        TS_FILES ${PROJECT_TS_FILES}
        NO_GENERATE_PLURALS_TS_FILE
    )

    # add qt executable for win32
    qt_add_executable(axserve_executable_win32
        MANUAL_FINALIZATION
    )
    target_link_libraries(axserve_executable_win32 PRIVATE axserve_common_win32)
    set_target_properties(axserve_executable_win32 PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_WIN32}"
    )
    list(APPEND AXSERVE_INSTALL_TARGETS axserve_executable_win32)

    # add qt executable for console
    qt_add_executable(axserve_executable_console
        MANUAL_FINALIZATION
    )
    target_link_libraries(axserve_executable_console PRIVATE axserve_common_console)
    set_target_properties(axserve_executable_console PROPERTIES
        WIN32_EXECUTABLE FALSE
        OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_CONSOLE}"
    )
    list(APPEND AXSERVE_INSTALL_TARGETS axserve_executable_console)

    # configure install dirs
    install(TARGETS ${AXSERVE_INSTALL_TARGETS}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # finalize qt executable for win32
    qt_finalize_executable(axserve_executable_win32)

    # finalize qt executable for console
    qt_finalize_executable(axserve_executable_console)

# endif build type branch, external vs normal
endif()
