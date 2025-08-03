# Copyright 2023 Yunseong Hwang
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

from __future__ import annotations

import win32api
import win32con
import win32job

from axserve.common.runnable_process import RunnableProcess


def create_job_object_for_cleanup(name: str | None = None) -> int:
    job_attributes = None
    job_name = name or ""
    job_handle = win32job.CreateJobObject(job_attributes, job_name)
    job_extended_info = win32job.QueryInformationJobObject(
        job_handle, win32job.JobObjectExtendedLimitInformation
    )
    job_extended_info_class = win32job.JobObjectExtendedLimitInformation
    job_basic_limit_information = job_extended_info["BasicLimitInformation"]
    job_basic_limit_information["LimitFlags"] = (
        win32job.JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
    )
    win32job.SetInformationJobObject(
        job_handle,
        job_extended_info_class,
        job_extended_info,
    )
    return job_handle


def assign_process_to_job_object(job_handle: int, process_id: int) -> None:
    if process_id == 0:
        msg = "Process id is zero"
        raise ValueError(msg)
    desired_access = win32con.PROCESS_TERMINATE | win32con.PROCESS_SET_QUOTA
    inherit_handle = False
    process_handle = win32api.OpenProcess(
        desired_access,
        inherit_handle,
        process_id,
    )
    return win32job.AssignProcessToJobObject(job_handle, process_handle)


class ManagedProcess(RunnableProcess):
    _job_handle: int = create_job_object_for_cleanup()

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        assign_process_to_job_object(self._job_handle, self.pid)


class ScopedProcess(ManagedProcess):
    _job_handle: int

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._job_handle = create_job_object_for_cleanup()
        assign_process_to_job_object(self._job_handle, self.pid)
