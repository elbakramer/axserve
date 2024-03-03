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
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations

import win32api
import win32con
import win32job

from axserve.common.runnable_popen import RunnablePopen


def CreateJobObjectForCleanUp() -> int:
    jobAttributes = None
    jobName = ""
    hJob = win32job.CreateJobObject(jobAttributes, jobName)
    extendedInfo = win32job.QueryInformationJobObject(hJob, win32job.JobObjectExtendedLimitInformation)
    basicLimitInformation = extendedInfo["BasicLimitInformation"]
    basicLimitInformation["LimitFlags"] = win32job.JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
    win32job.SetInformationJobObject(
        hJob,
        win32job.JobObjectExtendedLimitInformation,
        extendedInfo,
    )
    return hJob


def AssignProcessToJobObject(hJob: int, processId: int) -> None:
    assert processId != 0
    desiredAccess = win32con.PROCESS_TERMINATE | win32con.PROCESS_SET_QUOTA
    inheritHandle = False
    hProcess = win32api.OpenProcess(
        desiredAccess,
        inheritHandle,
        processId,
    )
    return win32job.AssignProcessToJobObject(hJob, hProcess)


class KillOnExitPopen(RunnablePopen):
    _job_handle: int = CreateJobObjectForCleanUp()

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        AssignProcessToJobObject(self._job_handle, self.pid)


class KillOnDeletePopen(RunnablePopen):
    _job_handle: int

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._job_handle = CreateJobObjectForCleanUp()
        AssignProcessToJobObject(self._job_handle, self.pid)
