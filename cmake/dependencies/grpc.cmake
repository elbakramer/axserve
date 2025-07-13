cmake_minimum_required(VERSION 3.27)

include_guard(GLOBAL)

message(CHECK_START "Checking gRPC provider")

if(AXSERVE_GRPC_PROVIDER STREQUAL "module")
    set(GRPC_EXTERNAL_NAME "gRPC")
    set(GRPC_PREFX_NAME "grpc")
    set(GRPC_THIRD_PARTY_NAME "grpc")

    set(GRPC_GIT_REPOSITORY "https://github.com/grpc/grpc.git")
    set(GRPC_GIT_TAG "v1.73.1")

    include("${CMAKE_CURRENT_LIST_DIR}/zlib.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/abseil-cpp.cmake")
    include("${CMAKE_CURRENT_LIST_DIR}/protobuf.cmake")

    if (BUILD_SHARED_LIBS)
        set(ZLIB_USE_STATIC_LIBS OFF)
        set(Protobuf_USE_STATIC_LIBS OFF)
    else()
        set(ZLIB_USE_STATIC_LIBS ON)
        set(Protobuf_USE_STATIC_LIBS ON)
    endif()

    string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" CMAKE_HOST_SYSTEM_PROCESSOR_LOWER)
    list(APPEND CMAKE_PROGRAM_PATH "${CMAKE_CURRENT_BINARY_DIR}/../${CMAKE_HOST_SYSTEM_PROCESSOR_LOWER}/${GRPC_PREFX_NAME}-release/bin")

    set(GRPC_CMAKE_CACHE_ARGS
        "-DCMAKE_PROGRAM_PATH:PATH=${CMAKE_PROGRAM_PATH}"
        "-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}"
        "-DCMAKE_SYSTEM_VERSION:STRING=${CMAKE_SYSTEM_VERSION}"
        "-DCMAKE_SYSTEM_PROCESSOR:STRING=${CMAKE_SYSTEM_PROCESSOR}"
        "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}"
        "-DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}"
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}"
        "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}"
        "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
        "-DgRPC_BUILD_TESTS:BOOL=${BUILD_TESTING}"
        "-DgRPC_INSTALL:BOOL=ON"
        "-DgRPC_ABSL_PROVIDER:STRING=package"
        "-DgRPC_CARES_PROVIDER:STRING=module"
        "-DgRPC_PROTOBUF_PROVIDER:STRING=package"
        "-DgRPC_RE2_PROVIDER:STRING=module"
        "-DgRPC_SSL_PROVIDER:STRING=module"
        "-DgRPC_ZLIB_PROVIDER:STRING=package"
        "-DProtobuf_USE_STATIC_LIBS:BOOL=${Protobuf_USE_STATIC_LIBS}"
        "-DZLIB_USE_STATIC_LIBS:BOOL=${ZLIB_USE_STATIC_LIBS}"
    )

    if(AXSERVE_ZLIB_PROVIDER STREQUAL "module")
        list(APPEND GRPC_DEPENDS ZLIB)
    endif()
    if(AXSERVE_ABSL_PROVIDER STREQUAL "module")
        list(APPEND GRPC_DEPENDS absl)
    endif()
    if(AXSERVE_PROTOBUF_PROVIDER STREQUAL "module")
        list(APPEND GRPC_DEPENDS Protobuf)
    endif()

    include("${CMAKE_CURRENT_LIST_DIR}/../util/external-project.cmake")

    ExternalProject_AddForThisProject("${GRPC_EXTERNAL_NAME}"
        PREFIX_NAME "${GRPC_PREFX_NAME}"
        THIRD_PARTY_NAME "${GRPC_THIRD_PARTY_NAME}"
        GIT_REPOSITORY "${GRPC_GIT_REPOSITORY}"
        GIT_TAG "${GRPC_GIT_TAG}"
        GIT_SUBMODULES_RECURSE FALSE
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        LOG_DOWNLOAD TRUE
        LOG_CONFIGURE TRUE
        LOG_BUILD TRUE
        CMAKE_ARGS ${GRPC_CMAKE_ARGS}
        CMAKE_CACHE_ARGS ${GRPC_CMAKE_CACHE_ARGS}
        DEPENDS ${GRPC_DEPENDS}
        # SKIP_BUILD TRUE
    )

    message(CHECK_PASS "${AXSERVE_GRPC_PROVIDER}")
elseif(AXSERVE_GRPC_PROVIDER STREQUAL "package")
    find_package(gRPC CONFIG REQUIRED)
    if(TARGET gRPC::grpc_cpp_plugin)
        set(GRPC_CPP_PLUGIN_EXECUTABLE "$<TARGET_FILE:gRPC::grpc_cpp_plugin>")
    else()
        find_program(GRPC_CPP_PLUGIN_EXECUTABLE REQUIRED NAMES grpc_cpp_plugin)
        add_executable(gRPC::grpc_cpp_plugin IMPORTED)
        set_target_properties(gRPC::grpc_cpp_plugin PROPERTIES
            IMPORTED_LOCATION "${GRPC_CPP_PLUGIN_EXECUTABLE}"
        )
    endif()
    message(CHECK_PASS "${AXSERVE_GRPC_PROVIDER}")
else()
    message(CHECK_FAIL "${AXSERVE_GRPC_PROVIDER}")
endif()
