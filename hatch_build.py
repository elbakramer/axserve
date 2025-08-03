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

import re
import shutil
import subprocess
import sys

from pathlib import Path
from typing import Any


try:
    from typing import override
except ImportError:
    from typing_extensions import override

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from packaging.requirements import Requirement


class CMakeBuildHook(BuildHookInterface):
    PLUGIN_NAME = "cmake"

    def check_command(self, cmd):
        return subprocess.check_call(cmd, shell=False)  # noqa: S603

    @override
    def clean(self, versions: list[str]) -> None:
        builds = self.config.get("builds", [])
        for build in builds:
            runtime_output_dir = build.get("runtime-output-dir")
            if not runtime_output_dir:
                continue
            runtime_output_dir = Path(runtime_output_dir)
            runtime_output_dir = runtime_output_dir.absolute()
            for filename in runtime_output_dir.iterdir():
                if filename.suffix == ".exe":
                    filename.unlink()

    @override
    def initialize(self, version: str, build_data: dict[str, Any]) -> None:
        builds = self.config.get("builds", [])
        for build in builds:
            source_dir = Path(build.get("source-dir", "."))
            build_dir = Path(build.get("build-dir", self.directory))
            build_subdir = build.get("build-subdir")
            if build_subdir:
                build_dir /= build_subdir
            generator = build.get("generator")
            config = build.get("config")
            system_processor = build.get("system-processor")
            toolchain = build.get("toolchain")
            runtime_output_dir = build.get("runtime-output-dir")
            library_output_dir = build.get("library-output-dir")
            cmake_args = build.get("args", [])
            build_parallel = build.get("build-parallel")
            output_files = build.get("output-files")
            marker = build.get("marker")
            if marker:
                marker = Requirement("dummy;" + marker).marker
            if marker and not marker.evaluate():
                continue
            if output_files:
                output_files = [Path(filename) for filename in output_files]
            if output_files and all(filename.exists() for filename in output_files):
                continue
            cmake_configure_cmd = [
                "cmake",
                "-S",
                str(source_dir),
                "-B",
                str(build_dir),
            ]
            if generator:
                cmake_configure_cmd += [
                    "-G",
                    generator,
                ]
            if toolchain:
                cmake_configure_cmd += [
                    "--toolchain",
                    toolchain,
                ]
            if config:
                cmake_configure_cmd += [
                    f"-DCMAKE_BUILD_TYPE:STRING={config}",
                ]
            if system_processor:
                cmake_configure_cmd += [
                    f"-DCMAKE_SYSTEM_PROCESSOR:STRING={system_processor}",
                ]
            if runtime_output_dir:
                runtime_output_dir = Path(runtime_output_dir)
                runtime_output_dir = runtime_output_dir.absolute()
                cmake_configure_cmd += [
                    f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH={runtime_output_dir}",
                ]
                if config:
                    cmake_configure_cmd += [
                        f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_{config.upper()}:PATH={runtime_output_dir}",
                    ]
            if library_output_dir:
                library_output_dir = Path(library_output_dir)
                library_output_dir = library_output_dir.absolute()
                cmake_configure_cmd += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH={library_output_dir}",
                ]
                if config:
                    cmake_configure_cmd += [
                        f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{config.upper()}:PATH={library_output_dir}",
                    ]
            if cmake_args:
                cmake_configure_cmd += cmake_args
            self.check_command(cmake_configure_cmd)
            cmake_build_cmd = [
                "cmake",
                "--build",
                str(build_dir),
            ]
            if config:
                cmake_build_cmd += ["--config", config]
            if build_parallel:
                if isinstance(build_parallel, bool):
                    cmake_build_cmd += ["--parallel"]
                elif isinstance(build_parallel, int):
                    cmake_build_cmd += [f"-j{build_parallel}"]
            self.check_command(cmake_build_cmd)

    @override
    def finalize(
        self,
        version: str,
        build_data: dict[str, Any],
        artifact_path: str,
    ) -> None:
        pass


class ProtocBuildHook(BuildHookInterface):
    PLUGIN_NAME = "protoc"

    def check_command(self, cmd):
        return subprocess.check_call(cmd, shell=False)  # noqa: S603

    def _fix_import_in_grpc_py(self, filename):
        with open(filename, "r+", encoding="utf-8") as f:
            data = f.read()
            data = re.sub(
                r"^((import)(\s+)([^\s\._]+)(_pb2)(.*))",
                r"from . \1",
                data,
                flags=re.MULTILINE,
            )
            f.seek(0)
            f.write(data)

    @override
    def dependencies(self) -> list[str]:
        return ["grpcio-tools>=1.73.1", "mypy-protobuf>=3.6.0"]

    @override
    def clean(self, versions: list[str]) -> None:
        pass

    @override
    def initialize(self, version: str, build_data: dict[str, Any]) -> None:
        protoc_cmds = [
            sys.executable,
            "-m",
            "grpc_tools.protoc",
        ]
        protoc_executable = "./build/amd64/protobuf-release/bin/protoc.exe"
        protoc_gen_grpc_python = "./build/amd64/grpc-release/bin/grpc_python_plugin.exe"
        protoc_gen_mypy = "protoc-gen-mypy"
        protoc_gen_mypy = shutil.which(protoc_gen_mypy)

        protoc_cmds = [
            protoc_executable,
            f"--plugin=protoc-gen-grpc_python={protoc_gen_grpc_python}",
            f"--plugin=protoc-gen-mypy={protoc_gen_mypy}",
        ]
        generates = self.config.get("generates", [])
        for generate in generates:
            sources = generate.get("sources", [])
            include_dirs = generate.get("include-dirs", [])
            outs = generate.get("outs", ["python", "grpc_python", "mypy", "mypy_grpc"])
            out_dir = generate.get("out-dir", ".")
            protoc_args = (
                [f"-I{i}" for i in include_dirs]
                + [f"--{out}_out={out_dir}" for out in outs]
                + sources
            )
            self.check_command(protoc_cmds + protoc_args)
            for source in sources:
                source_path = Path(source)
                grpc_py_filename = source_path.stem + "_pb2_grpc.py"
                grpc_py_filename = Path(out_dir) / grpc_py_filename
                if grpc_py_filename.exists():
                    self._fix_import_in_grpc_py(grpc_py_filename)
                grpc_pyi_filename = source_path.stem + "_pb2_grpc.pyi"
                grpc_pyi_filename = Path(out_dir) / grpc_pyi_filename
                if grpc_pyi_filename.exists():
                    self._fix_import_in_grpc_py(grpc_pyi_filename)

    @override
    def finalize(
        self,
        version: str,
        build_data: dict[str, Any],
        artifact_path: str,
    ) -> None:
        pass


class CustomBuildHook(BuildHookInterface):
    PLUGIN_NAME = "custom"

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        if hasattr(self, "_hooks"):
            msg = "Attribute already exists: _hooks"
            raise RuntimeError(msg)

        self._hooks: list[BuildHookInterface] = [
            CMakeBuildHook(*args, **kwargs),
            ProtocBuildHook(*args, **kwargs),
        ]

    @override
    def clean(self, versions: list[str]) -> None:
        for hook in self._hooks:
            hook.clean(versions)

    @override
    def initialize(self, version: str, build_data: dict[str, Any]) -> None:
        for hook in self._hooks:
            hook.initialize(version, build_data)

    @override
    def finalize(
        self,
        version: str,
        build_data: dict[str, Any],
        artifact_path: str,
    ) -> None:
        for hook in self._hooks:
            hook.finalize(version, build_data, artifact_path)


def get_build_hook():
    return CustomBuildHook
