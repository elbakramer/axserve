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
import re
import subprocess


PlatformSystem = platform.system()
PlatformBits, PlatformLinkage = platform.architecture()
PlatformMachine = platform.machine()

if PlatformSystem == "Windows":
    HostToWindowsMachineMapping = {
        "AMD64": "AMD64",
        "ARM64": "ARM64",
        "x86": "x86",
        "ARM": "ARM",
    }
    HostToWindows32BitMachineMapping = {
        "AMD64": "x86",
        "ARM64": "ARM",
    }
else:
    HostToWindowsMachineMapping = {
        "x86_64": "AMD64",
        "x86": "x86",
    }
    HostToWindows32BitMachineMapping = {
        "x86_64": "x86",
    }

WindowsMachine = HostToWindowsMachineMapping[PlatformMachine]
WindowsMachine32Bit = HostToWindows32BitMachineMapping[PlatformMachine]


def RegQuery(
    keyname: str | list[str],
    *,
    args: list[str] | None = None,
    bits: str | int | None = None,
) -> subprocess.CompletedProcess:
    if isinstance(keyname, list):
        keyname = "\\".join(keyname)

    if args is None:
        args = []

    if bits is None:
        bits, _ = platform.architecture()
    if isinstance(bits, int):
        if bits not in [64, 32]:
            msg = f"Invalid bits: {bits}"
            raise ValueError(msg)
        bits = f"{bits}bit"
    if isinstance(bits, str):
        if bits not in ["64bit", "32bit"]:
            msg = f"Invalid bits: {bits}"
            raise ValueError(msg)

    regview = {"64bit": "/reg:64", "32bit": "/reg:32"}[bits]

    if regview not in args:
        args.append(regview)

    args = ["reg.exe", "query", keyname, *args]
    completed = subprocess.run(
        args,
        capture_output=True,
        text=True,
        check=False,
    )
    return completed


def CheckRegQuery(
    keyname: str | list[str],
    *,
    args: list[str] | None = None,
    bits: str | int | None = None,
) -> bool:
    try:
        completed = RegQuery(keyname, args=args, bits=bits)
        completed.check_returncode()
    except subprocess.CalledProcessError:
        return False
    else:
        return True


def CheckRegQueryCLSID(clsid: str, bits: str | int | None = None) -> bool:
    keyname = ["HKCR", "CLSID", clsid]
    return CheckRegQuery(keyname, bits=bits)


def CheckRegQueryProgID(progid: str, bits: str | int | None = None) -> bool:
    keyname = ["HKCR", "CLSID"]
    args = ["/s", "/f", progid, "/d", "/e"]
    return CheckRegQuery(keyname, args=args, bits=bits)


def RegQueryProgID(progid: str, bits: str | int | None = None) -> subprocess.CompletedProcess:
    keyname = ["HKCR", "CLSID"]
    args = ["/s", "/f", progid, "/d", "/e"]
    return RegQuery(keyname, args=args, bits=bits)


def GetCLSIDFromString(s: str, bits: str | int | None = None) -> str | None:
    if CheckRegQueryCLSID(s, bits=bits):
        return s

    try:
        completed = RegQueryProgID(s, bits=bits)
        completed.check_returncode()
    except subprocess.CalledProcessError:
        return None
    else:
        path = ["HKEY_CLASSES_ROOT", "CLSID", "(\\{.+\\})", "ProgID"]
        pattern = "^" + "\\\\".join(path) + "$"
        match = re.search(
            pattern,
            completed.stdout,
            flags=re.MULTILINE,
        )
        if not match:
            return None
        clsid = match.group(1)
        return clsid


def CheckMachineFromCLSID(clsid: str) -> str | None:
    if CheckRegQueryCLSID(clsid):
        return WindowsMachine
    elif CheckRegQueryProgID(clsid):
        return WindowsMachine
    elif PlatformBits == "64bit":
        if CheckRegQueryCLSID(clsid, bits=32):
            return WindowsMachine32Bit
        elif CheckRegQueryProgID(clsid, bits=32):
            return WindowsMachine32Bit
    return None
