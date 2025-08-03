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

import click


@click.group()
@click.version_option()
def cli():
    pass


@cli.command(short_help="Start gRPC server process for an Active-X or COM support.")
@click.option("--machine", help="Machine type to run, AMD64 or x86")
@click.argument("args", nargs=-1, type=click.UNPROCESSED)
def serve(
    machine: str,
    args,
):
    import platform

    from axserve.common.process import ScopedProcess
    from axserve.server.process import find_server_executable_for_machine

    if not machine:
        machine = platform.machine()

    executable = find_server_executable_for_machine(machine)
    cmd = [str(executable), *args]

    process = ScopedProcess(cmd)
    process.run()


@cli.command(short_help="Generate python class code for client usage.")
@click.option(
    "--clsid",
    metavar="<CLSID>",
    required=True,
    help="CLSID for Active-X or COM.",
)
@click.option(
    "--filename",
    metavar="<PATH>",
    required=True,
    help="Path to output python module script.",
)
@click.option(
    "--async",
    "is_async",
    is_flag=True,
    help="Use asyncio syntax for asynchronous connection.",
)
def generate(
    clsid: str,
    filename: str,
    is_async: bool,  # noqa: FBT001
):
    import ast

    from pathlib import Path

    from axserve.client.stubgen import StubGenerator

    filepath = Path(filename)

    mod = StubGenerator(is_async=is_async).MakeStubModule(clsid)
    mod = ast.fix_missing_locations(mod)
    code = ast.unparse(mod)

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(code)


def main():
    cli()


if __name__ == "__main__":
    main()
