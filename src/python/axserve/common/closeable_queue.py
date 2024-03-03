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

from queue import Empty
from queue import Full
from queue import Queue
from time import time
from typing import Any
from typing import TypeVar

from axserve.common.closeable import Closeable


T = TypeVar("T")


class Closed(Exception):  # noqa: N818
    pass


class CloseableQueue(Queue[T], Closeable):
    def __init__(self, *args: Any, **kwargs: Any) -> None:
        super().__init__(*args, **kwargs)
        if hasattr(self, "_closed"):
            msg = "Object already has member _closed"
            raise RuntimeError(msg)
        self._closed = False

    def put(
        self,
        item: T,
        *,
        block: bool = True,
        timeout: float | None = None,
    ) -> None:
        with self.not_full:
            if self._closed:
                raise Closed
            if self.maxsize > 0:
                if not block:
                    if self._qsize() >= self.maxsize:
                        raise Full
                elif timeout is None:
                    while self._qsize() >= self.maxsize:
                        self.not_full.wait()
                elif timeout < 0:
                    msg = "'timeout' must be a non-negative number"
                    raise ValueError(msg)
                else:
                    endtime = time() + timeout
                    while self._qsize() >= self.maxsize:
                        remaining = endtime - time()
                        if remaining <= 0.0:  # noqa: PLR2004
                            raise Full
                        self.not_full.wait(remaining)
            self._put(item)
            self.unfinished_tasks += 1
            self.not_empty.notify()

    def get(
        self,
        *,
        block: bool = True,
        timeout: float | None = None,
    ) -> T:
        with self.not_empty:
            if not block:
                if not self._qsize():
                    raise Empty
            elif timeout is None:
                while not self._closed and not self._qsize():
                    self.not_empty.wait()
            elif timeout < 0:
                msg = "'timeout' must be a non-negative number"
                raise ValueError(msg)
            else:
                endtime = time() + timeout
                while not self._closed and not self._qsize():
                    remaining = endtime - time()
                    if remaining <= 0.0:  # noqa: PLR2004
                        raise Empty
                    self.not_empty.wait(remaining)
            if self._closed and not self._qsize():
                raise Closed
            item = self._get()
            self.not_full.notify()
            return item

    def close(
        self,
        *,
        block: bool = True,
        timeout: float | None = None,
        idempotent: bool = True,
        immediate: bool = False,
    ) -> None:
        with self.not_full:
            if self._closed:
                if idempotent:
                    return
                raise Closed
            if self.maxsize > 0:
                if not block:
                    if self._qsize() >= self.maxsize:
                        if not immediate:
                            raise Full
                elif timeout is None:
                    while self._qsize() >= self.maxsize:
                        if immediate:
                            break
                        self.not_full.wait()
                elif timeout < 0:
                    msg = "'timeout' must be a non-negative number"
                    raise ValueError(msg)
                else:
                    endtime = time() + timeout
                    while self._qsize() >= self.maxsize:
                        remaining = endtime - time()
                        if remaining <= 0.0:  # noqa: PLR2004
                            if immediate:
                                break
                            raise Full
                        self.not_full.wait(remaining)
            self._closed = True
            self.not_empty.notify_all()
            if immediate:
                self.not_full.notify_all()

    def closed(self) -> bool:
        with self.mutex:
            return self._closed
