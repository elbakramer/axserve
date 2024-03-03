# cmake version
cmake_minimum_required(VERSION 3.21)

# check host system
cmake_host_system_information(RESULT CMAKE_HOST_SYSTEM_NAME QUERY OS_NAME)
cmake_host_system_information(RESULT CMAKE_HOST_SYSTEM_PROCESSOR QUERY OS_PLATFORM)

# cache variables related to building the project executable
set(CMAKE_SYSTEM_PROCESSOR "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "Determines executable's target architecture, such as AMD64 or X86")
set(AXSERVE_WIN32_EXECUTABLE "" CACHE STRING "Determines executable's application type, such as GUI OR Console")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Determines executable's library linking type, such as Shared OR Static")
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Determines executable's build config type, such as Debug OR Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "Determines executable's output directory")

# cache variable for toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain/toolchain-msvc-auto.cmake" CACHE STRING "CMake toolchain file")

# project definition
project(AxServe VERSION 0.1.5 LANGUAGES CXX)

# cache variable for executable's name
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWERCASE)
set(AXSERVE_OUTPUT_NAME_WIN32_DEFAULT "${PROJECT_NAME_LOWERCASE}")
set(AXSERVE_OUTPUT_NAME_CONSOLE_DEFAULT "${PROJECT_NAME_LOWERCASE}")
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" CMAKE_SYSTEM_PROCESSOR_LOWERCASE)
string(APPEND AXSERVE_OUTPUT_NAME_WIN32_DEFAULT "-${CMAKE_SYSTEM_PROCESSOR_LOWERCASE}")
string(APPEND AXSERVE_OUTPUT_NAME_CONSOLE_DEFAULT "-${CMAKE_SYSTEM_PROCESSOR_LOWERCASE}")
string(APPEND AXSERVE_OUTPUT_NAME_WIN32_DEFAULT "-win32")
string(APPEND AXSERVE_OUTPUT_NAME_CONSOLE_DEFAULT "-console")
string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWERCASE)
string(APPEND AXSERVE_OUTPUT_NAME_WIN32_DEFAULT "-${CMAKE_BUILD_TYPE_LOWERCASE}")
string(APPEND AXSERVE_OUTPUT_NAME_CONSOLE_DEFAULT "-${CMAKE_BUILD_TYPE_LOWERCASE}")
set(AXSERVE_OUTPUT_NAME_WIN32 "${AXSERVE_OUTPUT_NAME_WIN32_DEFAULT}" CACHE STRING "Basename of win32 target executable, which is a filename without extension")
set(AXSERVE_OUTPUT_NAME_CONSOLE "${AXSERVE_OUTPUT_NAME_CONSOLE_DEFAULT}" CACHE STRING "Basename of console target executable, which is a filename without extension")
if(AXSERVE_WIN32_EXECUTABLE STREQUAL "" OR AXSERVE_WIN32_EXECUTABLE STREQUAL "BOTH" OR AXSERVE_WIN32_EXECUTABLE STREQUAL "ALL")
    set(AXSERVE_BUILD_BOTH_WIN32_EXECUTABLE TRUE)
else()
    set(AXSERVE_BUILD_BOTH_WIN32_EXECUTABLE FALSE)
endif()
if(AXSERVE_BUILD_BOTH_WIN32_EXECUTABLE)
    set(AXSERVE_OUTPUT_NAME_DEFAULT)
elseif(AXSERVE_WIN32_EXECUTABLE)
    set(AXSERVE_OUTPUT_NAME_DEFAULT "${AXSERVE_OUTPUT_NAME_WIN32}")
else()
    set(AXSERVE_OUTPUT_NAME_DEFAULT "${AXSERVE_OUTPUT_NAME_CONSOLE}")
endif()
set(AXSERVE_OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_DEFAULT}" CACHE STRING "Basename of target executable, which is a filename without extension")

# enumerate project files by type (source, proto, translation)
file(GLOB_RECURSE CXX_FILES
    "${CMAKE_CURRENT_LIST_DIR}/src/cpp/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/cpp/*.cc"
    "${CMAKE_CURRENT_LIST_DIR}/src/cpp/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/cpp/*.h.in"
)
file(GLOB_RECURSE PROTO_FILES
    "${CMAKE_CURRENT_LIST_DIR}/src/proto/*.proto"
)
file(GLOB_RECURSE TS_FILES
    "${CMAKE_CURRENT_LIST_DIR}/translations/*.ts"
)

# gather all project sources
set(PROJECT_SOURCE_FILES ${CXX_FILES} ${PROTO_FILES} ${TS_FILES})

# use cxx standard 17
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ compiler standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "C++ compiler standard required")

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

# configure dependent packages
message(CHECK_START "Configure dependencies")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/googletest.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/zlib.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/abseil-cpp.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/protobuf.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/grpc.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/qt.cmake")
list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "done")

# check dependency resolutions
set(AXSERVE_DEPENDS "")
if(AXSERVE_PROTOBUF_PROVIDER STREQUAL "module")
    list(APPEND AXSERVE_DEPENDS Protobuf)
endif()
if(AXSERVE_GRPC_PROVIDER STREQUAL "module")
    list(APPEND AXSERVE_DEPENDS gRPC)
endif()
if(AXSERVE_QT6_PROVIDER STREQUAL "module")
    list(APPEND AXSERVE_DEPENDS Qt6)
endif()

# if necessary, build with external projects
if(AXSERVE_DEPENDS)

    # print status for super build
    string(REPLACE ";" ", " AXSERVE_DEPENDS_PRINTABLE "${AXSERVE_DEPENDS}")
    message(STATUS "Configure to build ${PROJECT_NAME} with other deps including ${AXSERVE_DEPENDS_PRINTABLE} using ExternalProject")

    # name for external project
    set(AXSERVE_EXTERNAL_NAME "${PROJECT_NAME}")

    # directories for super build
    set(AXSERVE_SOURCE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}")
    set(AXSERVE_PREFIX_DIR   "${CMAKE_CURRENT_BINARY_DIR}/axserve")
    set(AXSERVE_BINARY_DIR   "${CMAKE_CURRENT_BINARY_DIR}/axserve-build")
    set(AXSERVE_INSTALL_DIR  "${AXSERVE_PREFIX_DIR}")
    set(AXSERVE_TMP_DIR      "${AXSERVE_PREFIX_DIR}/tmp")
    set(AXSERVE_STAMP_DIR    "${AXSERVE_TMP_DIR}/stamp")
    set(AXSERVE_LOG_DIR      "${AXSERVE_TMP_DIR}/log")
    set(AXSERVE_DOWNLOAD_DIR "${AXSERVE_TMP_DIR}/download")

    # setup cmake command and args
    set(AXSERVE_CMAKE_COMMAND "${CMAKE_COMMAND}")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(AXSERVE_CMAKE_ARGS
            "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
            "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
            "-P" "${CMAKE_CURRENT_LIST_DIR}/cmake/msvc/env-msvc.cmake"
            "--" "${AXSERVE_CMAKE_COMMAND}")
        set(AXSERVE_BUILD_COMMAND
            "${AXSERVE_CMAKE_COMMAND}" ${AXSERVE_CMAKE_ARGS}
            "--build" "${AXSERVE_BINARY_DIR}"
            "--parallel")
    endif()

    # cache args to pass
    set(AXSERVE_CMAKE_CACHE_ARGS
        "-DCMAKE_INSTALL_PREFIX:PATH=${AXSERVE_INSTALL_DIR}"
        "-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}"
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_CROSSCOMPILING:BOOL=${CMAKE_CROSSCOMPILING}"
        "-DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        "-DAXSERVE_WIN32_EXECUTABLE:STRING=${AXSERVE_WIN32_EXECUTABLE}"
        "-DAXSERVE_GTEST_PROVIDER:STRING=package"
        "-DAXSERVE_ZLIB_PROVIDER:STRING=package"
        "-DAXSERVE_ABSL_PROVIDER:STRING=package"
        "-DAXSERVE_PROTOBUF_PROVIDER:STRING=package"
        "-DAXSERVE_GRPC_PROVIDER:STRING=package"
        "-DAXSERVE_QT6_PROVIDER:STRING=package"
    )

    if(AXSERVE_BUILD_BOTH_WIN32_EXECUTABLE)

        # imported executable from the external project
        set(AXSERVE_EXECUTABLE_WIN32_IMPORTED_LOCATION "${AXSERVE_BINARY_DIR}/${AXSERVE_OUTPUT_NAME_WIN32}.exe")
        set(AXSERVE_EXECUTABLE_CONSOLE_IMPORTED_LOCATION "${AXSERVE_BINARY_DIR}/${AXSERVE_OUTPUT_NAME_CONSOLE}.exe")

        # add imported version of axserve executable
        add_executable(axserve_executable_win32 IMPORTED)
        set_target_properties(axserve_executable_win32 PROPERTIES
            IMPORTED_LOCATION "${AXSERVE_EXECUTABLE_WIN32_IMPORTED_LOCATION}"
            WIN32_EXECUTABLE TRUE
            OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_WIN32}"
        )
        add_executable(axserve_executable_console IMPORTED)
        set_target_properties(axserve_executable_console PROPERTIES
            IMPORTED_LOCATION "${AXSERVE_EXECUTABLE_CONSOLE_IMPORTED_LOCATION}"
            WIN32_EXECUTABLE FALSE
            OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_CONSOLE}"
        )

        set(AXSERVE_INSTALL_IMPORTED_RUNTIME_ARTIFACTS
            axserve_executable_win32
            axserve_executable_console
        )

    else()

        # imported executable from the external project
        set(AXSERVE_EXECUTABLE_IMPORTED_LOCATION "${AXSERVE_BINARY_DIR}/${AXSERVE_OUTPUT_NAME}.exe")

        # add imported version of axserve executable
        add_executable(axserve_executable IMPORTED)
        set_target_properties(axserve_executable PROPERTIES
            IMPORTED_LOCATION "${AXSERVE_EXECUTABLE_IMPORTED_LOCATION}"
            WIN32_EXECUTABLE "${AXSERVE_WIN32_EXECUTABLE}"
            OUTPUT_NAME "${AXSERVE_OUTPUT_NAME}"
        )
        set(AXSERVE_INSTALL_IMPORTED_RUNTIME_ARTIFACTS axserve_executable)


    endif()

    # external project for axserve inside super build
    include(ExternalProject)
    ExternalProject_Add("${AXSERVE_EXTERNAL_NAME}"
        PREFIX "${AXSERVE_PREFIX_DIR}"
        TMP_DIR "${AXSERVE_TMP_DIR}"
        STAMP_DIR "${AXSERVE_STAMP_DIR}"
        LOG_DIR "${AXSERVE_LOG_DIR}"
        DOWNLOAD_DIR "${AXSERVE_DOWNLOAD_DIR}"
        SOURCE_DIR "${AXSERVE_SOURCE_DIR}"
        BINARY_DIR "${AXSERVE_BINARY_DIR}"
        INSTALL_DIR "${AXSERVE_INSTALL_DIR}"
        GIT_SUBMODULES_RECURSE FALSE
        GIT_SHALLOW TRUE
        CMAKE_COMMAND "${AXSERVE_CMAKE_COMMAND}"
        CMAKE_CACHE_ARGS ${AXSERVE_CMAKE_CACHE_ARGS}
        BUILD_COMMAND ${AXSERVE_BUILD_COMMAND}
        BUILD_ALWAYS TRUE
        DEPENDS ${AXSERVE_DEPENDS}
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
    )

    # configure install dirs
    include(GNUInstallDirs)
    install(IMPORTED_RUNTIME_ARTIFACTS ${AXSERVE_INSTALL_IMPORTED_RUNTIME_ARTIFACTS}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

# else proceed with normal cmake lists
else()

    # print status for normal build
    message(STATUS "Configure to build ${PROJECT_NAME} just with the normal CMakeLists.txt")

    # set organization information for qsettings
    set(AXSERVE_ORG_NAME "Gennyous")
    set(AXSERVE_ORG_DOMAIN "gennyous.com")

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

    # configure header files
    if(AXSERVE_BUILD_BOTH_WIN32_EXECUTABLE)
        if(AXSERVE_OUTPUT_NAME_WIN32)
            set(AXSERVE_APP_NAME "${AXSERVE_OUTPUT_NAME_WIN32}")
            configure_file(
                "${CMAKE_CURRENT_LIST_DIR}/src/cpp/axserve/app/config.h.in"
                "${AXSERVE_CONFIGURE_OUTPUT_LOCATION}/win32/axserve/app/config.h"
            )
        endif()
        if(AXSERVE_OUTPUT_NAME_CONSOLE)
            set(AXSERVE_APP_NAME "${AXSERVE_OUTPUT_NAME_CONSOLE}")
            configure_file(
                "${CMAKE_CURRENT_LIST_DIR}/src/cpp/axserve/app/config.h.in"
                "${AXSERVE_CONFIGURE_OUTPUT_LOCATION}/console/axserve/app/config.h"
            )
        endif()
    elseif(AXSERVE_OUTPUT_NAME)
        set(AXSERVE_APP_NAME "${AXSERVE_OUTPUT_NAME}")
        configure_file(
            "${CMAKE_CURRENT_LIST_DIR}/src/cpp/axserve/app/config.h.in"
            "${AXSERVE_CONFIGURE_OUTPUT_LOCATION}/axserve/app/config.h"
        )
    endif()

    # add common proto library with protobuf generation
    add_library(axserve_proto ${PROTO_FILES})
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
    set(PROTO_IMPORT_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/src/proto"
    )
    protobuf_generate(
        TARGET axserve_proto
        LANGUAGE cpp
        IMPORT_DIRS ${PROTO_IMPORT_DIRS}
        PROTOC_OUT_DIR "${AXSERVE_PROTO_OUTPUT_LOCATION}"
    )
    protobuf_generate(
        TARGET axserve_proto
        LANGUAGE grpc
        IMPORT_DIRS ${PROTO_IMPORT_DIRS}
        PROTOC_OUT_DIR "${AXSERVE_PROTO_OUTPUT_LOCATION}"
        PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    )

    # set translation files output location
    set_source_files_properties(${TS_FILES} PROPERTIES
        OUTPUT_LOCATION "${AXSERVE_TRANSLATION_OUTPUT_LOCATION}"
    )

    # add common library
    if(AXSERVE_BUILD_BOTH_WIN32_EXECUTABLE)
        add_library(axserve_common)
        target_link_libraries(axserve_common PUBLIC
            Qt6::Widgets
            Qt6::Network
            Qt6::AxContainer
        )
        target_link_libraries(axserve_common PUBLIC
            axserve_proto
        )

        add_library(axserve_common_win32 ${PROJECT_SOURCE_FILES})
        target_link_libraries(axserve_common_win32 PUBLIC
            axserve_common
        )
        target_include_directories(axserve_common_win32 PUBLIC
            "${CMAKE_CURRENT_LIST_DIR}/src/cpp"
            "$<BUILD_INTERFACE:${AXSERVE_CONFIGURE_OUTPUT_LOCATION}>/win32"
        )
        qt_add_translations(axserve_common_win32
            TS_FILES ${TS_FILES}
        )

        add_library(axserve_common_console ${PROJECT_SOURCE_FILES})
        target_link_libraries(axserve_common_console PUBLIC
            axserve_common
        )
        target_include_directories(axserve_common_console PUBLIC
            "${CMAKE_CURRENT_LIST_DIR}/src/cpp"
            "$<BUILD_INTERFACE:${AXSERVE_CONFIGURE_OUTPUT_LOCATION}>/console"
        )
        qt_add_translations(axserve_common_console
            TS_FILES ${TS_FILES}
        )
    else()
        add_library(axserve_common ${PROJECT_SOURCE_FILES})
        target_link_libraries(axserve_common PUBLIC
            Qt6::Widgets
            Qt6::Network
            Qt6::AxContainer
        )
        target_link_libraries(axserve_common PUBLIC
            axserve_proto
        )
        target_include_directories(axserve_common PUBLIC
            "${CMAKE_CURRENT_LIST_DIR}/src/cpp"
            "$<BUILD_INTERFACE:${AXSERVE_CONFIGURE_OUTPUT_LOCATION}>"
        )
        qt_add_translations(axserve_common
            TS_FILES ${TS_FILES}
        )
    endif()

    # multi output case
    if(AXSERVE_BUILD_BOTH_WIN32_EXECUTABLE)

        # targets to install
        set(AXSERVE_INSTALL_TARGETS "")

        # add qt executable for win32
        if(AXSERVE_OUTPUT_NAME_WIN32)
            qt_add_executable(axserve_executable_win32
                MANUAL_FINALIZATION
            )
            target_link_libraries(axserve_executable_win32 PRIVATE axserve_common_win32)
            set_target_properties(axserve_executable_win32 PROPERTIES
                WIN32_EXECUTABLE TRUE
                OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_WIN32}"
            )
            list(APPEND AXSERVE_INSTALL_TARGETS axserve_executable_win32)
        endif()

        # add qt executable for console
        if(AXSERVE_OUTPUT_NAME_CONSOLE)
            qt_add_executable(axserve_executable_console
                MANUAL_FINALIZATION
            )
            target_link_libraries(axserve_executable_console PRIVATE axserve_common_console)
            set_target_properties(axserve_executable_console PROPERTIES
                WIN32_EXECUTABLE FALSE
                OUTPUT_NAME "${AXSERVE_OUTPUT_NAME_CONSOLE}"
            )
            list(APPEND AXSERVE_INSTALL_TARGETS axserve_executable_console)
        endif()

        # configure install dirs
        include(GNUInstallDirs)
        install(TARGETS ${AXSERVE_INSTALL_TARGETS}
            BUNDLE DESTINATION .
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )

        # finalize qt executable for win32
        if(AXSERVE_OUTPUT_NAME_WIN32)
            qt_finalize_executable(axserve_executable_win32)
        endif()

        # finalize qt executable for console
        if(AXSERVE_OUTPUT_NAME_CONSOLE)
            qt_finalize_executable(axserve_executable_console)
        endif()

    # single output case
    elseif(AXSERVE_OUTPUT_NAME)

        # add qt executable
        qt_add_executable(axserve_executable
            MANUAL_FINALIZATION
        )
        target_link_libraries(axserve_executable PRIVATE axserve_common)
        set_target_properties(axserve_executable PROPERTIES
            WIN32_EXECUTABLE "${AXSERVE_WIN32_EXECUTABLE}"
            OUTPUT_NAME "${AXSERVE_OUTPUT_NAME}"
        )

        # configure install dirs
        include(GNUInstallDirs)
        install(TARGETS axserve_executable
            BUNDLE DESTINATION .
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )

        # finalize qt executable
        qt_finalize_executable(axserve_executable)

    endif()

# endif build type branch, external vs normal
endif()
