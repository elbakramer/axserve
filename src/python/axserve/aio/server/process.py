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

import asyncio

from asyncio import Task
from asyncio.subprocess import Process
from collections.abc import Coroutine
from typing import Any

from axserve.common.process import AssignProcessToJobObject
from axserve.common.process import CreateJobObjectForCleanUp
from axserve.server.process import FindServerExecutableForCLSID


class AxServeServerProcess(Process):
    _init_coro: Coroutine[Any, Any, Process]
    _init_task: Task

    _underlying_proc: Process
    _job_handle: int

    def __init__(self, clsid: str, address: str, **kwargs):
        self._init_coro = self.__ainit__(clsid, address, **kwargs)
        self._init_task = asyncio.create_task(self._init_coro)

    async def __ainit__(self, clsid: str, address: str, **kwargs):
        executable = FindServerExecutableForCLSID(clsid)
        cmd = [executable, f"--clsid={clsid}", f"--address={address}", "--no-gui"]
        self._underlying_proc = await asyncio.create_subprocess_exec(*cmd, **kwargs)
        super().__init__(
            self._underlying_proc._transport,
            self._underlying_proc._protocol,
            self._underlying_proc._loop,
        )
        self._job_handle = CreateJobObjectForCleanUp()
        AssignProcessToJobObject(self._job_handle, self.pid)

    async def __aenter__(self):
        await self._init_task
        return self

    async def __aexit__(self, exc_type, exc_value, traceback):
        return


async def CreateAxServeServerProcess(clsid: str, address: str, *args, **kwargs) -> AxServeServerProcess:
    proc = AxServeServerProcess(clsid, address, *args, **kwargs)
    return await proc.__aenter__()
