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

from asyncio import Queue
from asyncio import QueueEmpty
from asyncio import QueueFull
from typing import Any
from typing import TypeVar


try:
    from typing import override
except ImportError:
    from typing_extensions import override

from axserve.aio.common.async_closeable import AsyncCloseable
from axserve.common.closeable_queue import Closed as QueueClosed


T = TypeVar("T")


class AsyncCloseableQueue(Queue[T], AsyncCloseable):
    def __init__(self, *args: Any, **kwargs: Any) -> None:
        super().__init__(*args, **kwargs)
        if hasattr(self, "_closed"):
            msg = "Object already has member named: _closed"
            raise RuntimeError(msg)
        self._closed = False

    @override
    async def put(self, item):
        """Put an item into the queue.

        Put an item into the queue. If the queue is full, wait until a free
        slot is available before adding item.
        """
        while self.full():
            putter = self._get_loop().create_future()  # type: ignore
            self._putters.append(putter)  # type: ignore
            try:
                await putter
            except:
                putter.cancel()  # Just in case putter is not done yet.
                try:  # noqa: SIM105
                    # Clean self._putters from canceled putters.
                    self._putters.remove(putter)  # type: ignore
                except ValueError:
                    # The putter could be removed from self._putters by a
                    # previous get_nowait call.
                    pass
                if not self.full() and not putter.cancelled():
                    # We were woken up by get_nowait(), but can't take
                    # the call.  Wake up the next in line.
                    self._wakeup_next(self._putters)  # type: ignore
                raise
        return self.put_nowait(item)

    @override
    def put_nowait(self, item):
        """Put an item into the queue without blocking.

        If no free slot is immediately available, raise QueueFull.
        """
        if self._closed:
            raise QueueClosed
        if self.full():
            raise QueueFull
        self._put(item)
        self._unfinished_tasks += 1
        self._finished.clear()  # type: ignore
        self._wakeup_next(self._getters)  # type: ignore

    @override
    async def get(self):
        """Remove and return an item from the queue.

        If queue is empty, wait until an item is available.
        """
        while not self._closed and self.empty():
            getter = self._get_loop().create_future()  # type: ignore
            self._getters.append(getter)  # type: ignore
            try:
                await getter
            except:
                getter.cancel()  # Just in case getter is not done yet.
                try:  # noqa: SIM105
                    # Clean self._getters from canceled getters.
                    self._getters.remove(getter)  # type: ignore
                except ValueError:
                    # The getter could be removed from self._getters by a
                    # previous put_nowait call.
                    pass
                if not self.empty() and not getter.cancelled():
                    # We were woken up by put_nowait(), but can't take
                    # the call.  Wake up the next in line.
                    self._wakeup_next(self._getters)  # type: ignore
                raise
        return self.get_nowait()

    @override
    def get_nowait(self):
        """Remove and return an item from the queue.

        Return an item if one is immediately available, else raise QueueEmpty.
        """
        if self._closed and self.empty():
            raise QueueClosed
        if self.empty():
            raise QueueEmpty
        item = self._get()
        self._wakeup_next(self._putters)  # type: ignore
        return item

    async def close(self):
        """Close the queue.

        Close the queue. If the queue is full, wait until a free
        slot is available before closing.
        """
        while self.full():
            putter = self._get_loop().create_future()  # type: ignore
            self._putters.append(putter)  # type: ignore
            try:
                await putter
            except:
                putter.cancel()  # Just in case putter is not done yet.
                try:  # noqa: SIM105
                    # Clean self._putters from canceled putters.
                    self._putters.remove(putter)  # type: ignore
                except ValueError:
                    # The putter could be removed from self._putters by a
                    # previous get_nowait call.
                    pass
                if not self.full() and not putter.cancelled():
                    # We were woken up by get_nowait(), but can't take
                    # the call.  Wake up the next in line.
                    self._wakeup_next(self._putters)  # type: ignore
                raise
        return self.close_nowait()

    def close_nowait(self):
        """Close the queue without blocking."""
        self._closed = True
        self._wakeup_next(self._getters)  # type: ignore

    def closed(self) -> bool:
        """Return True if the queue is closed, False otherwise."""
        return self._closed
