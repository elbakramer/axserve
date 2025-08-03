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
# SPDX-FileCopyrightText: 2025 Yunseong Hwang
#
# SPDX-License-Identifier: Apache-2.0

from __future__ import annotations

import platform
import struct
import winreg

import click


PE_MACHINE_MAPPING = {
    0x8664: "AMD64",
    0x14C: "X86",
    0xAA64: "ARM64",
    0x1C0: "ARM",
}

VIEW_FLAGS_MAPPING = {
    "64bit": [winreg.KEY_WOW64_64KEY, winreg.KEY_WOW64_32KEY],
    "32bit": [0],
}


def clsid_from_progid(progid: str, view_flag: int = 0) -> str | None:
    key_path = rf"{progid}\CLSID"
    try:
        with winreg.OpenKey(
            winreg.HKEY_CLASSES_ROOT,
            key_path,
            0,
            winreg.KEY_READ | view_flag,
        ) as key:
            value, _ = winreg.QueryValueEx(key, None)  # type: ignore
            return value
    except FileNotFoundError:
        return None


def normalize_identifier(identifier: str, view_flag: int = 0) -> str:
    if identifier.startswith("{"):
        return identifier
    clsid = clsid_from_progid(identifier, view_flag)
    if not clsid:
        msg = f"Invalid ProgID: {identifier}"
        raise ValueError(msg)
    return clsid


def get_server_path(clsid: str, subkey: str, view_flag: int = 0) -> str | None:
    key_path = rf"CLSID\{clsid}\{subkey}"
    try:
        with winreg.OpenKey(
            winreg.HKEY_CLASSES_ROOT,
            key_path,
            0,
            winreg.KEY_READ | view_flag,
        ) as key:
            value, _ = winreg.QueryValueEx(key, None)  # type: ignore
            return value.strip('"')
    except FileNotFoundError:
        return None


def get_machine_from_pe(path: str) -> str | None:
    try:
        with open(path, "rb") as f:
            f.seek(0x3C)
            offset = struct.unpack("<I", f.read(4))[0]
            f.seek(offset + 4)
            machine = struct.unpack("<H", f.read(2))[0]
            return PE_MACHINE_MAPPING.get(machine)
    except OSError:
        return None


def check_machine_for_clsid(identifier: str) -> str | None:
    bits, _ = platform.architecture()

    views = VIEW_FLAGS_MAPPING.get(bits, [0])
    subkeys = ["LocalServer32", "InprocServer32"]

    for view_flag in views:
        for subkey in subkeys:
            clsid = normalize_identifier(identifier, view_flag)
            path = get_server_path(clsid, subkey, view_flag)
            if not path:
                continue
            machine = get_machine_from_pe(path)
            if not machine:
                continue
            return machine

    return None


@click.command()
@click.argument("clsid")
def main(clsid: str):
    machine = check_machine_for_clsid(clsid)
    if machine:
        click.echo(machine)


if __name__ == "__main__":
    main()
