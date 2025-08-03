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

if(NOT DEFINED CMAKE_CROSSCOMPILING)
    if(CMAKE_SYSTEM_NAME AND CMAKE_HOST_SYSTEM_NAME AND NOT CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
        set(CMAKE_CROSSCOMPILING ON)
    elseif(CMAKE_SYSTEM_PROCESSOR AND CMAKE_HOST_SYSTEM_PROCESSOR AND NOT CMAKE_SYSTEM_PROCESSOR STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR)
        set(CMAKE_CROSSCOMPILING ON)
    else()
        set(CMAKE_CROSSCOMPILING OFF)
    endif()
endif()

string(APPEND CMAKE_C_FLAGS_INIT   " -utf-8")
string(APPEND CMAKE_CXX_FLAGS_INIT " -utf-8")

include("${CMAKE_CURRENT_LIST_DIR}/setup-msvc-vars.cmake")
setup_msvc_vars(OPTIONAL)
