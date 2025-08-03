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
import platform

from asyncio.subprocess import Process
from typing import TYPE_CHECKING
from typing import Any

from axserve.aio.common.async_initializable import AsyncInitializable
from axserve.common.process import assign_process_to_job_object
from axserve.common.process import create_job_object_for_cleanup
from axserve.server.process import find_server_executable_for_machine


if TYPE_CHECKING:
    from pathlib import Path


class AxServeServerProcess(Process, AsyncInitializable["AxServeServerProcess"]):
    _address: str
    _machine: str

    _program: Path
    _args: list[str]
    _kwargs: dict[str, Any]

    _underlying_proc: Process
    _job_handle: int

    def __init__(
        self,
        address: str,
        *,
        machine: str | None = None,
        **kwargs,
    ):
        if not machine:
            machine = platform.machine()

        self._address = address
        self._machine = machine

        self._program = find_server_executable_for_machine(self._machine)
        self._args = ["--preset", "service", "--address-uri", address]
        self._kwargs = kwargs

    async def __ainit__(self):
        self._underlying_proc = await asyncio.create_subprocess_exec(
            self._program,
            *self._args,
            **self._kwargs,
        )
        Process.__init__(
            self,
            self._underlying_proc._transport,  # type: ignore
            self._underlying_proc._protocol,  # type: ignore
            self._underlying_proc._loop,  # type: ignore
        )
        self._job_handle = create_job_object_for_cleanup()
        assign_process_to_job_object(self._job_handle, self.pid)

    async def __afinalize__(self):
        self._underlying_proc.terminate()
        await self._underlying_proc.wait()
