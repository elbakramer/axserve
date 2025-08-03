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

from axserve.common.process import ScopedProcess
from axserve.common.registry import check_machine_for_clsid


EXECUTABLE_DIR = Path(__file__).parent / "exe"


def find_server_executable_for_machine(machine: str) -> Path:
    name = f"axserve-console-{machine.lower()}.exe"
    executable = EXECUTABLE_DIR / name
    if not executable.exists():
        msg = f"Cannot find server executable for machine: {machine}"
        raise RuntimeError(msg)
    return executable


def find_server_executable_for_clsid(clsid: str) -> Path:
    machine = check_machine_for_clsid(clsid)
    if not machine:
        msg = f"Cannot determine machine type for clsid: {clsid}"
        raise ValueError(msg)
    return find_server_executable_for_machine(machine)


class AxServeServerProcess(ScopedProcess):
    def __init__(
        self,
        address: str,
        *,
        machine: str | None = None,
        **kwargs,
    ):
        if not machine:
            machine = platform.machine()
        executable = find_server_executable_for_machine(machine)
        cmd = [executable, "--preset", "service", "--address-uri", address]
        super().__init__(cmd, **kwargs)
