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

from collections.abc import AsyncIterable
from collections.abc import AsyncIterator
from typing import TypeVar

from axserve.aio.common.async_closeable_queue import AsyncCloseableQueue
from axserve.aio.common.async_closeable_queue import QueueClosed


T = TypeVar("T")


class AsyncIterableQueue(AsyncCloseableQueue[T], AsyncIterable[T]):
    async def next(self) -> T:  # noqa: A003
        try:
            return await self.get()
        except QueueClosed as exc:
            raise StopAsyncIteration from exc

    def __aiter__(self) -> AsyncIterator[T]:
        return self

    async def __anext__(self) -> T:
        return await self.next()
