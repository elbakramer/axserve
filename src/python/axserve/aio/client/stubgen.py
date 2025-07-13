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

from axserve.client.stubgen import *  # type: ignore # noqa: F403
from axserve.client.stubgen import StubGenerator as SyncStubGenerator


class StubGenerator(SyncStubGenerator):
    def __init__(
        self,
        *,
        is_base: bool = False,
    ):
        is_async = True
        super().__init__(
            is_async=is_async,
            is_base=is_base,
        )
