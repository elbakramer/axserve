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

from subprocess import CompletedProcess
from subprocess import Popen
from subprocess import TimeoutExpired
from typing import AnyStr


def run_process(
    process: Popen,
    input: AnyStr | None = None,  # noqa: A002
    timeout: float | None = None,
    *,
    check: bool | None = None,
) -> CompletedProcess:
    if check is None:
        check = False

    with process:
        try:
            stdout, stderr = process.communicate(input, timeout=timeout)
        except TimeoutExpired as exc:
            process.kill()
            exc.stdout, exc.stderr = process.communicate()
            raise
        except:
            process.kill()
            raise
        completed = CompletedProcess(
            process.args,
            process.returncode,
            stdout,
            stderr,
        )
        if check:
            completed.check_returncode()
        return completed


class RunnableProcess(Popen):
    def run(
        self,
        input: AnyStr | None = None,  # noqa: A002
        timeout: float | None = None,
        *,
        check: bool | None = None,
    ):
        return run_process(self, input, timeout, check=check)
