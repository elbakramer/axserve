from __future__ import annotations

import subprocess

from pathlib import Path

from axserve.common.process import AssignProcessToJobObject
from axserve.common.process import CreateJobObjectForCleanUp
from axserve.common.registry import CheckMachineForCLSID


class AxServeServerProcess(subprocess.Popen):
    def _find_executable(self, clsid):
        configs = ["debug", "release"]
        machine = CheckMachineForCLSID(clsid)
        if not machine:
            raise ValueError("Invalid clsid given")
        executable_candidate_names = [
            f"axserve-{machine.lower()}-console-{config}.exe" for config in configs
        ]
        executable_dir = Path(__file__).parent / "exe"
        for name in executable_candidate_names:
            executable = executable_dir / name
            if executable.exists():
                break
        if not executable.exists():
            raise RuntimeError("Cannot find server executable")
        return executable

    def __init__(self, clsid, address, *args, **kwargs):
        executable = self._find_executable(clsid)
        cmd = [executable, f"--clsid={clsid}", f"--address={address}", "--no-gui"]
        super().__init__(cmd, *args, **kwargs)
        self._job_handle = CreateJobObjectForCleanUp()
        AssignProcessToJobObject(self._job_handle, self.pid)
