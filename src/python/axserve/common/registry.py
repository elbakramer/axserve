from __future__ import annotations

import platform

import pywintypes
import win32api

from win32api import RegOpenKey
from win32con import HKEY_CLASSES_ROOT


def CheckMachineForCLSID(clsid) -> str | None:
    try:
        clsid = pywintypes.IID(clsid)
    except pywintypes.com_error:
        return None
    host_machine = platform.machine()
    try:
        root = RegOpenKey(HKEY_CLASSES_ROOT, "WOW6432Node")
        clsids = RegOpenKey(root, "CLSID")
        RegOpenKey(clsids, str(clsid))
        return {
            "AMD64": "x86",
            "ARM64": "ARM",
        }[host_machine]
    except win32api.error:
        pass
    try:
        root = HKEY_CLASSES_ROOT
        clsids = RegOpenKey(root, "CLSID")
        RegOpenKey(clsids, str(clsid))
        return host_machine
    except win32api.error:
        pass
    return None
