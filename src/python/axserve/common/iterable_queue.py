from __future__ import annotations

from typing import Iterable
from typing import Iterator
from typing import Optional
from typing import TypeVar

from axserve.common.closeable_queue import CloseableQueue
from axserve.common.closeable_queue import Closed


T = TypeVar("T")


class IterableQueue(CloseableQueue[T], Iterable[T]):
    def next(self, timeout: Optional[float] = None) -> T:
        try:
            return self.get(timeout=timeout)
        except Closed as exc:
            raise StopIteration from exc

    def __iter__(self) -> Iterator[T]:
        return self

    def __next__(self) -> T:
        return self.next()
