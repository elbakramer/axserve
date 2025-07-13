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

import asyncio

from abc import abstractmethod
from asyncio import Event
from asyncio import Lock
from asyncio import Task
from collections.abc import Coroutine
from collections.abc import Mapping
from types import TracebackType
from typing import Any
from typing import Protocol


class AsyncInitializableInternals:
    _initializing: Lock
    _initialized: Event

    _coroutine: Coroutine[Any, Any, None]
    _task: Task[None]

    def __init__(
        self,
        instance: AsyncInitializable,
        args: tuple[str],
        kwargs: Mapping[str, Any],
    ):
        self._initializing = Lock()
        self._initialized = Event()

        self._coroutine = self.initialize(instance, args, kwargs)
        self._task = asyncio.create_task(self._coroutine)

    async def initialize(
        self,
        instance: AsyncInitializable,
        args: tuple[str],
        kwargs: Mapping[str, Any],
    ) -> None:
        async with self._initializing:
            self._initialized.clear()
            await instance.__ainit__(
                *args,
                **kwargs,
            )
            self._initialized.set()


class AsyncInitializable(Protocol):
    __async__: AsyncInitializableInternals

    @abstractmethod
    async def __ainit__(self, *args, **kwargs) -> None:
        raise NotImplementedError()

    def __init__(self, *args, **kwargs) -> None:
        self.__async__ = AsyncInitializableInternals(self, args, kwargs)

    async def __aenter__(self):
        await self.__async__._initialized.wait()
        return self

    async def __aexit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        exc_traceback: TracebackType | None,
    ):
        return
