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
import re
import struct
import subprocess

import click


PE_MACHINE_MAPPING = {
    0x8664: "AMD64",
    0x14C: "X86",
    0xAA64: "ARM64",
    0x1C0: "ARM",
}

VIEW_FLAGS_MAPPING = {
    "64bit": ["/reg:64", "/reg:32"],
    "32bit": [None],
}


reg_query_value_pattern = re.compile(r"^\s+(\S+)\s+(\S+)\s+(.+)$")


def parse_reg_query_output(output: str):
    path = None
    items = []
    lines = output.split("\n")
    for line in lines:
        if not line:
            continue
        if not line[0].isspace():
            path = line
        else:
            m = reg_query_value_pattern.match(line)
            if not m:
                continue
            key = m.group(1)
            typ = m.group(2)
            val = m.group(3)
            item = (key, typ, val)
            items.append(item)
    return path, items


def reg_query_value(
    key_path: str,
    key_value: str | None = None,
    view_flag: str | None = None,
) -> str | None:
    cmd = ["wine", "reg.exe", "query", key_path]
    if key_value:
        cmd.append("/v")
        cmd.append(key_value)
    else:
        cmd.append("/ve")
    if view_flag:
        cmd.append(view_flag)
    try:
        output = subprocess.check_output(cmd, text=True, shell=False)  # noqa: S603
    except subprocess.CalledProcessError:
        return None
    _, items = parse_reg_query_output(output)
    if not key_value:
        value = items[0][2]
    else:
        values = {item[0]: item[2] for item in items}
        value = values.get(key_value)
    return value


def clsid_from_progid(progid: str, view_flag: str | None = None) -> str | None:
    key_path = rf"HKCR\{progid}\CLSID"
    return reg_query_value(key_path, view_flag=view_flag)


def normalize_identifier(identifier: str, view_flag: str | None = None) -> str:
    if identifier.startswith("{"):
        return identifier
    clsid = clsid_from_progid(identifier, view_flag)
    if not clsid:
        msg = f"Invalid ProgID: {identifier}"
        raise ValueError(msg)
    return clsid


def get_server_path(
    clsid: str, subkey: str, view_flag: str | None = None
) -> str | None:
    key_path = rf"HKCR\CLSID\{clsid}\{subkey}"
    return reg_query_value(key_path, view_flag=view_flag)


def convert_path(path: str) -> str | None:
    cmd = ["winepath", "--unix", path]
    try:
        output = subprocess.check_output(cmd, text=True, shell=False)  # noqa: S603
    except subprocess.CalledProcessError:
        return None
    return output.strip()


def get_machine_from_pe(path: str) -> str | None:
    unix_path = convert_path(path)
    if not unix_path:
        return None
    try:
        with open(unix_path, "rb") as f:
            f.seek(0x3C)
            offset = struct.unpack("<I", f.read(4))[0]
            f.seek(offset + 4)
            machine = struct.unpack("<H", f.read(2))[0]
            return PE_MACHINE_MAPPING.get(machine)
    except OSError:
        return None


def check_machine_for_clsid(identifier: str) -> str | None:
    bits, _ = platform.architecture()

    views = VIEW_FLAGS_MAPPING.get(bits, [None])
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
