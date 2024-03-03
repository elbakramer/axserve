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

import click


@click.group()
@click.version_option()
def cli():
    pass


@cli.command(short_help="Start gRPC server process for an Active-X or COM support.")
@click.option(
    "--clsid",
    metavar="<CLSID>",
    required=True,
    help="CLSID for Active-X or COM.",
)
@click.option(
    "--address",
    metavar="<ADDRESS>",
    required=True,
    help="Address URI for gRPC server to bind.",
)
@click.option(
    "--tray-icon",
    is_flag=True,
    help="Create system tray icon for process management.",
)
@click.option(
    "--hidden",
    is_flag=True,
    help="Hide the starting window on start. Valid only when the tray icon is created.",
)
@click.option(
    "--no-gui",
    is_flag=True,
    help="Disable GUI components. Valid only when console is attached.",
)
@click.option(
    "--translate",
    is_flag=True,
    help="Translate to current locale if available.",
)
@click.option(
    "--log-level",
    metavar="<TYPE>",
    type=click.Choice(["debug", "info", "warning", "critical", "fatal"], case_sensitive=False),
    help="Mininmum log level or type to print.",
)
def serve(
    clsid: str,
    address: str,
    tray_icon: bool,
    hidden: bool,
    no_gui: bool,
    translate: bool,
    log_level: str,
):
    from axserve.common.process import KillOnDeletePopen
    from axserve.server.process import FindServerExecutableForCLSID

    executable = FindServerExecutableForCLSID(clsid)
    cmd = [executable, f"--clsid={clsid}", f"--address={address}"]

    if tray_icon:
        cmd.append("--tray-icon")
    if hidden:
        cmd.append("--hidden")
    if no_gui:
        cmd.append("--no-gui")
    if translate:
        cmd.append("--translate")
    if log_level:
        cmd.extend(["--log-level", log_level])

    process = KillOnDeletePopen(cmd)
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
    "--use-asyncio",
    is_flag=True,
    help="Use asyncio syntax for asynchronous connection.",
)
def generate(
    clsid: str,
    filename: str,
    use_asyncio: bool,
):
    import ast

    from pathlib import Path

    from axserve.client.stubgen import StubGenerator

    filepath = Path(filename)

    mod = StubGenerator(is_async=use_asyncio).MakeStubModule(clsid)
    mod = ast.fix_missing_locations(mod)
    code = ast.unparse(mod)

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(code)


def main():
    cli()


if __name__ == "__main__":
    main()
