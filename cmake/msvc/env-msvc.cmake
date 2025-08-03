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

block(SCOPE_FOR POLICIES)
cmake_policy(SET CMP0007 NEW)

if(NOT CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
    message(DEBUG "Not running in script mode")
    return()
endif()

set(_CMAKE_ARGS)
foreach(_INDEX RANGE "${CMAKE_ARGC}")
    list(APPEND _CMAKE_ARGS "${CMAKE_ARGV${_INDEX}}")
endforeach()

set(_CMAKE_UNPARSED_SEP "--")
list(FIND _CMAKE_ARGS "${_CMAKE_UNPARSED_SEP}" _CMAKE_UNPARSED_SEP_LOC)

if(_CMAKE_UNPARSED_SEP_LOC LESS 0)
    message(DEBUG "No arguments given")
    return()
endif()

math(EXPR _CMAKE_UNPARSED_BEGIN "${_CMAKE_UNPARSED_SEP_LOC}+1")
math(EXPR _CMAKE_UNPARSED_LENGTH "${CMAKE_ARGC}-${_CMAKE_UNPARSED_SEP_LOC}")

list(SUBLIST _CMAKE_ARGS "${_CMAKE_UNPARSED_BEGIN}" "${_CMAKE_UNPARSED_LENGTH}" _CMAKE_UNPARSED_ARGS)

if(NOT _CMAKE_UNPARSED_ARGS)
    message(DEBUG "No arguments given")
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/setup-msvc-vars.cmake")
setup_msvc_vars()

execute_process(
    COMMAND ${_CMAKE_UNPARSED_ARGS}
    COMMAND_ERROR_IS_FATAL ANY
)

endblock()
