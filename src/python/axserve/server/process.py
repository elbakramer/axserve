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

import platform

from pathlib import Path

from axserve.common.process import KillOnDeletePopen
from axserve.common.registry import CheckMachineFromCLSID


EXECUTABLE_DIR = Path(__file__).parent / "exe"


def FindServerExecutableForMachine(machine: str) -> Path:
    name = f"axserve-console-{machine.lower()}.exe"
    executable = EXECUTABLE_DIR / name
    if not executable.exists():
        msg = f"Cannot find server executable for machine: {machine}"
        raise RuntimeError(msg)
    return executable


def FindServerExecutableForCLSID(clsid: str) -> Path:
    machine = CheckMachineFromCLSID(clsid)
    if not machine:
        msg = f"Cannot determine machine type for clsid: {clsid}"
        raise ValueError(msg)
    return FindServerExecutableForMachine(machine)


class AxServeServerProcess(KillOnDeletePopen):
    def __init__(
        self,
        address: str,
        *,
        machine: str | None = None,
        **kwargs,
    ):
        if not machine:
            machine = platform.machine()
        executable = FindServerExecutableForMachine(machine)
        cmd = [executable, "--preset", "service", "--address-uri", address]
        super().__init__(cmd, **kwargs)
