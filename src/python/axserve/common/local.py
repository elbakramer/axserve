from __future__ import annotations

import asyncio
import typing

from abc import ABC
from abc import abstractmethod
from asyncio import AbstractEventLoop
from contextlib import contextmanager
from threading import RLock
from threading import local
from typing import TYPE_CHECKING
from typing import Any
from weakref import WeakKeyDictionary


if TYPE_CHECKING:
    from collections.abc import Iterable
    from collections.abc import Mapping
    from collections.abc import MutableMapping


class Local(ABC):
    @abstractmethod
    def __getattribute__(self, name):
        raise object.__getattribute__(self, name)

    @abstractmethod
    def __setattr__(self, name, value):
        raise NotImplementedError

    @abstractmethod
    def __delattr__(self, name):
        raise NotImplementedError


class ThreadLocal(local, Local): ...


class _LoopLocalImpl:
    _dicts: WeakKeyDictionary[AbstractEventLoop, MutableMapping[str, Any]]
    _args: Iterable[Any]
    _kwargs: Mapping[str, Any]
    _lock: RLock

    def __init__(self, args, kwargs):
        self._dicts = WeakKeyDictionary()
        self._args = args
        self._kwargs = kwargs
        self._lock = RLock()

    def get_dict(self):
        loop = asyncio.get_running_loop()
        return self._dicts[loop]

    def create_dict(self):
        loop = asyncio.get_running_loop()
        self._dicts[loop] = {}
        return self._dicts[loop]

    @classmethod
    @contextmanager
    def patch(cls, self):
        impl = object.__getattribute__(self, "_local__impl")
        impl = typing.cast(_LoopLocalImpl, impl)
        try:
            dct = impl.get_dict()
        except KeyError:
            dct = impl.create_dict()
            args = impl._args
            kwargs = impl._kwargs
            self.__init__(*args, **kwargs)
        with impl._lock:
            object.__setattr__(self, "__dict__", dct)
            yield


class LoopLocal(Local):
    __slots__ = ("_local__impl",)

    def __new__(cls, /, *args, **kwargs):
        if (args or kwargs) and (cls.__init__ is object.__init__):
            msg = "Initialization arguments are not supported"
            raise TypeError(msg)
        self = object.__new__(cls)
        impl = _LoopLocalImpl(args, kwargs)
        object.__setattr__(self, "_local__impl", impl)
        impl.create_dict()
        return self

    def __getattribute__(self, name):
        with _LoopLocalImpl.patch(self):
            return object.__getattribute__(self, name)

    def __setattr__(self, name, value):
        if name == "__dict__":
            msg = (
                f"{self.__class__.__name__!r} object attribute '__dict__' is read-only"
            )
            raise AttributeError(msg)
        with _LoopLocalImpl.patch(self):
            return object.__setattr__(self, name, value)

    def __delattr__(self, name):
        if name == "__dict__":
            msg = (
                f"{self.__class__.__name__!r} object attribute '__dict__' is read-only"
            )
            raise AttributeError(msg)
        with _LoopLocalImpl.patch(self):
            return object.__delattr__(self, name)
