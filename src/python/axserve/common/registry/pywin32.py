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

import functools
import platform

import pywintypes
import win32api

from win32api import RegOpenKey
from win32con import HKEY_CLASSES_ROOT
from win32con import HKEY_CURRENT_CONFIG
from win32con import HKEY_CURRENT_USER
from win32con import HKEY_LOCAL_MACHINE
from win32con import HKEY_USERS


PlatformSystem = platform.system()
PlatformBits, PlatformLinkage = platform.architecture()
PlatformMachine = platform.machine()

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

WindowsMachine = HostToWindowsMachineMapping[PlatformMachine]
WindowsMachine32Bit = HostToWindows32BitMachineMapping[PlatformMachine]

RootKeyMapping = {
    "HKLM": HKEY_LOCAL_MACHINE,
    "HKCU": HKEY_CURRENT_USER,
    "HKCR": HKEY_CLASSES_ROOT,
    "HKU": HKEY_USERS,
    "HKCC": HKEY_CURRENT_CONFIG,
}


def CheckRegQuery(
    keyname: str | list[str],
    *,
    bits: str | int | None = None,
) -> bool:
    if isinstance(keyname, str):
        if keyname.startswith("\\\\"):
            msg = "Given keyname starts with explicit computername"
            raise ValueError(msg)
        keyname = keyname.split("\\")
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
    keyname[0] = RootKeyMapping[keyname[0]]
    if PlatformBits == "64bit" and bits == "32bit" and "WOW6432Node" not in keyname:
        keyname.insert(keyname.index("CLSID"), "WOW6432Node")
    try:
        functools.reduce(RegOpenKey, keyname)
    except win32api.error:
        return False
    else:
        return True


def CheckRegQueryCLSID(clsid: str, bits: str | int | None = None) -> bool:
    try:
        clsid = pywintypes.IID(clsid)
    except pywintypes.com_error:
        return False
    keyname = ["HKCR", "CLSID", str(clsid)]
    return CheckRegQuery(keyname, bits=bits)


def CheckRegQueryProgID(progid: str, bits: str | int | None = None) -> bool:
    try:
        clsid = pywintypes.IID(progid)
    except pywintypes.com_error:
        return False
    keyname = ["HKCR", "CLSID", str(clsid)]
    return CheckRegQuery(keyname, bits=bits)


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
