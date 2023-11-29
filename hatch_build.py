from __future__ import annotations

import re
import subprocess
import sys

from pathlib import Path
from typing import Any

from hatchling.builders.hooks.plugin.interface import BuildHookInterface
from packaging.requirements import Requirement


class BuildHookInterfaceWithCheckCommand(BuildHookInterface):
    def check_command(self, cmd):
        return subprocess.check_call(cmd)


class CMakeBuildHook(BuildHookInterfaceWithCheckCommand):
    PLUGIN_NAME = "cmake"

    def clean(self, versions: list[str]) -> None:
        pass

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
            if config:
                cmake_configure_cmd += [
                    f"-DCMAKE_BUILD_TYPE:STRING={config}",
                ]
            if system_processor:
                cmake_configure_cmd += [
                    f"-DCMAKE_SYSTEM_PROCESSOR:STRING={system_processor}",
                ]
            if runtime_output_dir:
                cmake_configure_cmd += [
                    f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH={runtime_output_dir}",
                ]
                if config:
                    cmake_configure_cmd += [
                        f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_{config.upper()}:PATH={runtime_output_dir}",
                    ]
            if library_output_dir:
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

    def finalize(self, version: str, build_data: dict[str, Any], artifact_path: str) -> None:
        pass


class ProtocBuildHook(BuildHookInterfaceWithCheckCommand):
    PLUGIN_NAME = "protoc"

    def _fix_import_in_grpc_py(self, filename):
        with open(filename, "r+", encoding="utf-8") as f:
            data = f.read()
            data = re.sub(
                r"^((import)(\s+)([^\s\._]+)(_pb2)(\s+)(as))",
                r"from . \1",
                data,
                flags=re.MULTILINE,
            )
            f.seek(0)
            f.write(data)

    def clean(self, versions: list[str]) -> None:
        pass

    def initialize(self, version: str, build_data: dict[str, Any]) -> None:
        generates = self.config.get("generates", [])
        for generate in generates:
            sources = generate.get("sources", [])
            include_dirs = generate.get("include-dirs", [])
            outs = generate.get("outs", ["python", "pyi", "grpc_python"])
            out_dir = generate.get("out-dir", ".")
            protoc_args = [f"-I{i}" for i in include_dirs] + [f"--{out}_out={out_dir}" for out in outs] + sources
            self.check_command([sys.executable, "-m", "grpc_tools.protoc", *protoc_args])
            for source in sources:
                source_path = Path(source)
                grpc_py_filename = source_path.stem + "_pb2_grpc.py"
                grpc_py_filename = Path(out_dir) / grpc_py_filename
                if grpc_py_filename.exists():
                    self._fix_import_in_grpc_py(grpc_py_filename)

    def finalize(self, version: str, build_data: dict[str, Any], artifact_path: str) -> None:
        pass


class CustomBuildHook(BuildHookInterface):
    PLUGIN_NAME = "custom"

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        assert not hasattr(self, "_hooks")
        self._hooks: list[BuildHookInterface] = [
            ProtocBuildHook(*args, **kwargs),
            CMakeBuildHook(*args, **kwargs),
        ]

    def clean(self, versions: list[str]) -> None:
        for hook in self._hooks:
            hook.clean(versions)

    def initialize(self, version: str, build_data: dict[str, Any]) -> None:
        for hook in self._hooks:
            hook.initialize(version, build_data)

    def finalize(self, version: str, build_data: dict[str, Any], artifact_path: str) -> None:
        for hook in self._hooks:
            hook.finalize(version, build_data, artifact_path)


def get_build_hook():
    return CustomBuildHook
