from __future__ import annotations

from abc import ABC
from abc import abstractmethod


class Closeable(ABC):
    @abstractmethod
    def close(self) -> None:
        raise NotImplementedError()

    @classmethod
    def __subclasshook__(cls, __subclass: type) -> bool:
        if cls is Closeable:
            if any(
                "close" in __baseclass.__dict__ for __baseclass in __subclass.__mro__
            ):
                return True
        return super().__subclasshook__(__subclass)
