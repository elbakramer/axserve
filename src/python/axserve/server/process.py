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

from pathlib import Path

from axserve.common.process import KillOnDeletePopen
from axserve.common.registry import CheckMachineFromCLSID
from axserve.common.runnable_popen import RunnablePopen


def FindServerExecutableForCLSID(clsid: str) -> Path:
    configs = ["debug", "release"]
    machine = CheckMachineFromCLSID(clsid)
    if not machine:
        msg = f"Cannot determine machine type for clsid: {clsid}"
        raise ValueError(msg)
    executable_candidate_names = [f"axserve-{machine.lower()}-console-{config}.exe" for config in configs]
    executable_dir = Path(__file__).parent / "exe"
    executable: Path | None = None
    for name in executable_candidate_names:
        executable = executable_dir / name
        if executable.exists():
            break
    if not executable or not executable.exists():
        msg = f"Cannot find server executable for clsid: {clsid}"
        raise RuntimeError(msg)
    return executable


class AxServeServerProcess(KillOnDeletePopen, RunnablePopen):
    def __init__(self, clsid: str, address: str, *args, **kwargs):
        executable = FindServerExecutableForCLSID(clsid)
        cmd = [executable, f"--clsid={clsid}", f"--address={address}", "--no-gui"]
        super().__init__(cmd, *args, **kwargs)
