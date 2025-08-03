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

from collections.abc import Awaitable
from contextlib import AbstractAsyncContextManager
from typing import TYPE_CHECKING
from typing import Protocol
from typing import TypeVar


if TYPE_CHECKING:
    from types import TracebackType


T_co = TypeVar("T_co", covariant=True)


class AsyncInitializable(
    AbstractAsyncContextManager[T_co],
    Awaitable[T_co],
    Protocol[T_co],
):
    async def __ainit__(self):
        return

    async def __afinalize__(self):
        return

    async def __aenter__(self):  # type: ignore
        await self.__ainit__()
        return self

    async def __aexit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        exc_traceback: TracebackType | None,
    ):
        await self.__afinalize__()

    def __await__(self):  # type: ignore
        return self.__aenter__().__await__()
