from __future__ import annotations

from queue import Empty
from queue import Full
from queue import Queue
from time import time
from typing import Any
from typing import Optional
from typing import TypeVar

from axserve.common.closeable import Closeable


T = TypeVar("T")


class Closed(Exception):
    pass


class CloseableQueue(Queue[T], Closeable):
    def __init__(self, *args: Any, **kwargs: Any) -> None:
        super().__init__(*args, **kwargs)
        assert not hasattr(self, "_closed")
        self._closed = False

    def put(
        self,
        item: T,
        block: bool = True,
        timeout: Optional[float] = None,
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
                    raise ValueError("'timeout' must be a non-negative number")
                else:
                    endtime = time() + timeout
                    while self._qsize() >= self.maxsize:
                        remaining = endtime - time()
                        if remaining <= 0.0:
                            raise Full
                        self.not_full.wait(remaining)
            self._put(item)
            self.unfinished_tasks += 1  # pylint: disable=no-member
            self.not_empty.notify()

    def get(
        self,
        block: bool = True,
        timeout: Optional[float] = None,
    ) -> T:
        with self.not_empty:
            if self._closed and not self._qsize():
                raise Closed
            if not block:
                if not self._qsize():
                    raise Empty
            elif timeout is None:
                while not self._qsize():
                    self.not_empty.wait()
            elif timeout < 0:
                raise ValueError("'timeout' must be a non-negative number")
            else:
                endtime = time() + timeout
                while not self._qsize():
                    remaining = endtime - time()
                    if remaining <= 0.0:
                        raise Empty
                    self.not_empty.wait(remaining)
            item = self._get()
            self.not_full.notify()
            return item

    def close(
        self,
        block: bool = True,
        timeout: Optional[float] = None,
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
                    raise ValueError("'timeout' must be a non-negative number")
                else:
                    endtime = time() + timeout
                    while self._qsize() >= self.maxsize:
                        remaining = endtime - time()
                        if remaining <= 0.0:
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
