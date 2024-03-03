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
import functools
import inspect
import typing

from asyncio import Task
from collections.abc import Awaitable
from collections.abc import Callable
from collections.abc import Sequence
from typing import Any
from typing import Concatenate
from typing import Generic
from typing import ParamSpec
from typing import TypeVar
from typing import overload

from wrapt import ObjectProxy

from axserve.aio.client.abstract import AbstractAxServeObject
from axserve.aio.common.async_connectable import AsyncConnectable
from axserve.proto import active_pb2
from axserve.proto.variant_conversion import AnnotationFromTypeName
from axserve.proto.variant_conversion import ValueFromVariant
from axserve.proto.variant_conversion import ValueToVariant


T = TypeVar("T")
U = TypeVar("U")
R = TypeVar("R")

P = ParamSpec("P")
Q = ParamSpec("Q")


class AxServePropertyType(ObjectProxy, Generic[T]):
    def __init__(self, prop: AxServeProperty[T], instance: AbstractAxServeObject) -> None:
        super().__init__(prop)
        self._self_prop = prop
        self._self_instance = instance

    def get(self) -> T:
        return self._self_prop._get(self._self_instance)

    def set(self, value: T) -> active_pb2.SetPropertyResponse:  # noqa: A003
        return self._self_prop._set(self._self_instance, value)


class AxServeProperty(Generic[T]):
    @overload
    def __init__(self) -> None:
        ...

    @overload
    def __init__(self, index: int) -> None:
        ...

    @overload
    def __init__(self, info: active_pb2.PropertyInfo) -> None:
        ...

    @overload
    def __init__(self, f: Callable[[Any], T]) -> None:
        ...

    def __init__(self, arg: int | active_pb2.PropertyInfo | Callable[[Any], Awaitable[T]] | None = None) -> None:
        self._index: int | None = None
        self._name: str | None = None
        self._info: active_pb2.PropertyInfo | None = None

        if isinstance(arg, int):
            self._index = arg
        elif isinstance(arg, active_pb2.PropertyInfo):
            self._set_info(arg)
        elif callable(arg):
            functools.update_wrapper(self, arg)
            self._name = arg.__name__
        elif arg is not None:
            msg = f"Invalid argument: {arg!r}"
            raise ValueError(msg)

    def __set_name__(self, owner, name: str) -> None:
        self._name = name

    def _set_info(self, info: active_pb2.PropertyInfo) -> None:
        self._index = info.index
        self._name = info.name
        self._info = info

    def _get_index(self, instance: AbstractAxServeObject) -> int:
        if self._index is not None:
            return self._index
        if self._name is not None and instance._has_member_name(self._name):
            return instance._get_member_by_name(self._name)._property._index
        msg = "Cannot specify index"
        raise ValueError(msg)

    async def _get(
        self,
        instance: AbstractAxServeObject | None = None,
        owner: type | None = None,  # noqa: ARG002
    ) -> AxServeProperty[T] | T:
        if instance is None:
            return self
        await instance._initialized()
        if self._info is not None and not self._info.is_readable and self._info.is_writable:
            msg = "This property is not readable"
            raise AttributeError(msg)
        index = self._get_index(instance)
        stub = instance._get_stub()
        current_handle_event = instance._get_current_handle_event()
        request = active_pb2.GetPropertyRequest()
        request.index = index
        if current_handle_event:
            request_context = active_pb2.RequestContext.EVENT_CALLBACK
            callback_event_index = current_handle_event.index
            request.request_context = request_context
            request.callback_event_index = callback_event_index
        response = await stub.GetProperty(request)
        response = typing.cast(active_pb2.GetPropertyResponse, response)
        return ValueFromVariant(response.value)

    async def _set(
        self,
        instance: AbstractAxServeObject,
        value: T,
    ) -> active_pb2.SetPropertyResponse:
        await instance._initialized()
        if self._info is not None and not self._info.is_writable and self._info.is_readable:
            msg = "This property is not writable"
            raise AttributeError(msg)
        index = self._get_index(instance)
        stub = instance._get_stub()
        current_handle_event = instance._get_current_handle_event()
        request = active_pb2.SetPropertyRequest()
        request.index = index
        ValueToVariant(value, request.value)
        if current_handle_event:
            request_context = active_pb2.RequestContext.EVENT_CALLBACK
            callback_event_index = current_handle_event.index
            request.request_context = request_context
            request.callback_event_index = callback_event_index
        response = await stub.SetProperty(request)
        response = typing.cast(active_pb2.SetPropertyResponse, response)
        return response

    @overload
    def __get__(self, instance: Any, owner: type | None = None) -> Awaitable[T]:
        ...

    @overload
    def __get__(self, instance: None, owner: type) -> AxServeProperty[T]:
        ...

    def __get__(
        self,
        instance: AbstractAxServeObject | None = None,
        owner: type | None = None,
    ) -> AxServeProperty[T] | Awaitable[T]:
        if instance is None:
            return self
        return self._get(instance, owner)

    def __set__(
        self,
        instance: AbstractAxServeObject,
        value: T,
    ) -> Task[active_pb2.SetPropertyResponse]:
        coro = self._set(instance, value)
        task = asyncio.create_task(coro)
        return task


class AxServeMethodType(ObjectProxy, Generic[P, R], Callable[P, Awaitable[R]]):
    def __init__(self, func: AxServeMethod[P, R], instance: AbstractAxServeObject) -> None:
        super().__init__(func)
        self._self_func = func
        self._self_instance = instance

    def __call__(self, *args: P.args, **kwargs: P.kwargs) -> Awaitable[R]:
        return self._self_func(self._self_instance, *args, **kwargs)

    def call(self, *args: P.args, **kwargs: P.kwargs) -> Awaitable[R]:
        return self.__call__(*args, **kwargs)


class AxServeMethod(Generic[P, R], Callable[Concatenate[Any, P], Awaitable[R]]):
    @overload
    def __init__(self) -> None:
        ...

    @overload
    def __init__(self, index: int) -> None:
        ...

    @overload
    def __init__(self, info: active_pb2.MethodInfo) -> None:
        ...

    @overload
    def __init__(self, f: Callable[Concatenate[Any, P], Awaitable[R]]) -> None:
        ...

    def __init__(
        self, arg: int | active_pb2.MethodInfo | Callable[Concatenate[Any, P], Awaitable[R]] | None = None
    ) -> None:
        self._index: int | None = None
        self._name: str | None = None
        self._signature: inspect.Signature | None = None
        self._info: active_pb2.MethodInfo | None = None

        if isinstance(arg, int):
            self._index = arg
        elif isinstance(arg, active_pb2.MethodInfo):
            self._set_info(arg)
        elif callable(arg):
            functools.update_wrapper(self, arg)
            self._name = arg.__name__
            self._signature = inspect.signature(functools.partial(arg, None))
        elif arg is not None:
            msg = f"Invalid argument: {arg!r}"
            raise ValueError(msg)

    def __set_name__(self, owner, name: str) -> None:
        self._name = name

    def _set_info(self, info: active_pb2.MethodInfo) -> None:
        self._index = info.index
        self._name = info.name
        self._signature = inspect.Signature(
            parameters=[
                inspect.Parameter(
                    name=arg.name,
                    kind=inspect.Parameter.POSITIONAL_OR_KEYWORD,
                    annotation=AnnotationFromTypeName(arg.argument_type),
                )
                for arg in info.arguments
            ],
            return_annotation=AnnotationFromTypeName(info.return_type),
        )
        self._info = info

    def _get_index(self, instance: AbstractAxServeObject) -> int:
        if self._index is not None:
            return self._index
        if self._name is not None and instance._has_member_name(self._name):
            return instance._get_member_by_name(self._name)._method._index
        msg = "Cannot specify index"
        raise ValueError(msg)

    def _bind_args(self, *args, **kwargs) -> Sequence[Any]:
        if not self._signature:
            return args
        bound_args = self._signature.bind(*args, **kwargs)
        bound_args.apply_defaults()
        return bound_args.args

    async def _call(self, instance: AbstractAxServeObject, *args: P.args, **kwargs: P.kwargs) -> R:
        await instance._initialized()
        index = self._get_index(instance)
        stub = instance._get_stub()
        current_handle_event = instance._get_current_handle_event()
        request = active_pb2.InvokeMethodRequest()
        request.index = index
        bound_args = self._bind_args(*args, **kwargs)
        for arg in bound_args:
            ValueToVariant(arg, request.arguments.add())
        if current_handle_event:
            request_context = active_pb2.RequestContext.EVENT_CALLBACK
            callback_event_index = current_handle_event.index
            request.request_context = request_context
            request.callback_event_index = callback_event_index
        response = await stub.InvokeMethod(request)
        response = typing.cast(active_pb2.InvokeMethodResponse, response)
        return ValueFromVariant(response.return_value)

    def __call__(self, instance: AbstractAxServeObject, *args: P.args, **kwargs: P.kwargs) -> Awaitable[R]:
        return self._call(instance, *args, **kwargs)

    @overload
    def __get__(self, instance: Any, owner: type | None = None) -> AxServeMethodType[P, R]:
        ...

    @overload
    def __get__(self, instance: None, owner: type) -> AxServeMethod[P, R]:
        ...

    def __get__(
        self,
        instance: AbstractAxServeObject | None = None,
        owner: type | None = None,
    ) -> AxServeMethod[P, R] | AxServeMethodType[P, R]:
        if instance is None:
            return self
        return AxServeMethodType(self, instance)


class AxServeEventType(ObjectProxy, Generic[P], Callable[P, Awaitable[None]], AsyncConnectable[P]):
    def __init__(self, func: AxServeEvent[P], instance: AbstractAxServeObject) -> None:
        super().__init__(func)
        self._self_func = func
        self._self_instance = instance

    def __call__(self, *args: P.args, **kwargs: P.kwargs) -> Awaitable[None]:
        return self._self_func(self._self_instance, *args, **kwargs)

    def call(self, *args: P.args, **kwargs: P.kwargs) -> Awaitable[None]:
        return self.__call__(*args, **kwargs)

    async def connect(self, handler: Callable[P, Any]) -> active_pb2.ConnectEventResponse | None:
        response = None
        instance = self._self_instance
        await instance._initialized()
        index = self._self_func._get_index(instance)
        handlers = instance._get_event_handlers(index)
        handlers_lock = instance._get_event_handlers_lock(index)
        async with handlers_lock:
            if not handlers:
                stub = instance._get_stub()
                current_handle_event = instance._get_current_handle_event()
                request = active_pb2.ConnectEventRequest()
                request.index = index
                if current_handle_event:
                    request_context = active_pb2.RequestContext.EVENT_CALLBACK
                    callback_event_index = current_handle_event.index
                    request.request_context = request_context
                    request.callback_event_index = callback_event_index
                response = await stub.ConnectEvent(request)
                response = typing.cast(active_pb2.ConnectEventResponse, response)
                if not response.successful:
                    msg = "Failed to connect event"
                    raise RuntimeError(msg)
            handlers.append(handler)
        return response

    async def disconnect(self, handler: Callable[P, Any]) -> active_pb2.DisconnectEventResponse | None:
        response = None
        instance = self._self_instance
        await instance._initialized()
        index = self._self_func._get_index(instance)
        handlers = instance._get_event_handlers(index)
        handlers_lock = instance._get_event_handlers_lock(index)
        async with handlers_lock:
            handlers.remove(handler)
            if not handlers:
                stub = instance._get_stub()
                current_handle_event = instance._get_current_handle_event()
                request = active_pb2.DisconnectEventRequest()
                request.index = index
                if current_handle_event:
                    request_context = active_pb2.RequestContext.EVENT_CALLBACK
                    callback_event_index = current_handle_event.index
                    request.request_context = request_context
                    request.callback_event_index = callback_event_index
                response = await stub.DisconnectEvent(request)
                response = typing.cast(active_pb2.DisconnectEventResponse, response)
                if not response.successful:
                    msg = "Failed to disconnect event"
                    raise RuntimeError(msg)
        return response


class AxServeEvent(Generic[P], Callable[Concatenate[Any, P], Awaitable[None]]):
    @overload
    def __init__(self) -> None:
        ...

    @overload
    def __init__(self, index: int) -> None:
        ...

    @overload
    def __init__(self, info: active_pb2.EventInfo) -> None:
        ...

    @overload
    def __init__(self, f: Callable[Concatenate[Any, P], Awaitable[None]]) -> None:
        ...

    def __init__(
        self, arg: int | active_pb2.EventInfo | Callable[Concatenate[Any, P], Awaitable[None]] | None = None
    ) -> None:
        self._index: int | None = None
        self._name: str | None = None
        self._signature: inspect.Signature | None = None
        self._info: active_pb2.EventInfo | None = None

        if isinstance(arg, int):
            self._index = arg
        elif isinstance(arg, active_pb2.EventInfo):
            self._set_info(arg)
        elif callable(arg):
            functools.update_wrapper(self, arg)
            self._name = arg.__name__
            self._signature = inspect.signature(functools.partial(arg, None))
        elif arg is not None:
            msg = f"Invalid argument: {arg!r}"
            raise ValueError(msg)

    def __set_name__(self, owner, name: str) -> None:
        self._name = name

    def _set_info(self, info: active_pb2.EventInfo) -> None:
        self._index = info.index
        self._name = info.name
        self._signature = inspect.Signature(
            parameters=[
                inspect.Parameter(
                    name=arg.name,
                    kind=inspect.Parameter.POSITIONAL_OR_KEYWORD,
                    annotation=AnnotationFromTypeName(arg.argument_type),
                )
                for arg in info.arguments
            ],
        )
        self._info = info

    def _get_index(self, instance: AbstractAxServeObject) -> int:
        if self._index is not None:
            return self._index
        if self._name is not None and instance._has_member_name(self._name):
            return instance._get_member_by_name(self._name)._event._index
        msg = "Cannot specify index"
        raise ValueError(msg)

    async def _call(self, instance: AbstractAxServeObject, *args: P.args, **kwargs: P.kwargs) -> None:
        await instance._initialized()
        index = self._get_index(instance)
        handlers = instance._get_event_handlers(index)
        handlers_lock = instance._get_event_handlers_lock(index)
        async with handlers_lock:
            handlers = list(handlers)
        for handler in handlers:
            res = handler(*args, **kwargs)
            if inspect.isawaitable(res):
                await res

    def __call__(self, instance: AbstractAxServeObject, *args: P.args, **kwargs: P.kwargs) -> Awaitable[None]:
        return self._call(instance, *args, **kwargs)

    @overload
    def __get__(self, instance: Any, owner: type | None = None) -> AxServeEventType[P]:
        ...

    @overload
    def __get__(self, instance: None, owner: type) -> AxServeEvent[P]:
        ...

    def __get__(
        self,
        instance: AbstractAxServeObject | None = None,
        owner: type | None = None,
    ) -> AxServeEvent[P] | AxServeEventType[P]:
        if instance is None:
            return self
        return AxServeEventType(self, instance)


class AxServeMemberType(ObjectProxy, Generic[T, P, R, Q], AsyncConnectable[Q]):
    def __init__(self, member: AxServeMember[T, P, R, Q], instance: AbstractAxServeObject) -> None:
        super().__init__(member)
        self._self_mem = member
        self._self_instance = instance

    @property
    def prop(self) -> AxServePropertyType[T]:
        if self._self_mem._property:
            return AxServePropertyType(self._self_mem._property, self._self_instance)
        raise NotImplementedError()

    @property
    def method(self) -> AxServeMethodType[P, R]:
        if self._self_mem._method:
            return self._self_mem._method.__get__(self._self_instance)
        raise NotImplementedError()

    @property
    def event(self) -> AxServeEventType[Q]:
        if self._self_mem._event:
            return self._self_mem._event.__get__(self._self_instance)
        raise NotImplementedError()

    async def get(self) -> T:
        return await self.prop.get()

    async def set(self, value: T) -> active_pb2.SetPropertyResponse:  # noqa: A003
        return await self.prop.set(value)

    def __call__(self, *args, **kwargs) -> Awaitable[R] | Awaitable[None]:
        return self._self_mem(self._self_instance, *args, **kwargs)

    async def call(self, *args, **kwargs) -> R | None:
        return await self.__call__(*args, **kwargs)

    async def connect(self, handler: Callable[P, Any]) -> active_pb2.ConnectEventResponse | None:
        return await self.event.connect(handler)

    async def disconnect(self, handler: Callable[P, Any]) -> active_pb2.DisconnectEventResponse | None:
        return await self.event.disconnect(handler)


class AxServeMember(Generic[T, P, R, Q]):
    def __init__(
        self,
        *items: AxServeProperty[T] | AxServeMethod[P, R] | AxServeEvent[Q],
        prop: AxServeProperty[T] | None = None,
        method: AxServeMethod[P, R] | None = None,
        event: AxServeEvent[Q] | None = None,
    ) -> None:
        self._property: AxServeProperty[T] | None = None
        self._method: AxServeMethod[P, R] | None = None
        self._event: AxServeEvent[Q] | None = None

        for item in items:
            if isinstance(item, AxServeProperty):
                self._property = item
            elif isinstance(item, AxServeMethod):
                self._method = method
            elif isinstance(item, AxServeEvent):
                self._event = event
            else:
                msg = f"Invalid item given: {item!r}"
                raise ValueError(msg)

        if prop is not None:
            self._property = prop
        if method is not None:
            self._method = method
        if event is not None:
            self._event = event

    def __set_name__(self, owner, name: str) -> None:
        self._name = name

    def _set_info(self, info: active_pb2.PropertyInfo | active_pb2.MethodInfo | active_pb2.EventInfo) -> None:
        if hasattr(info, "property_type"):
            if self._property:
                self._property._set_info(info)
        elif hasattr(info, "return_type"):
            if self._method:
                self._method._set_info(info)
        elif hasattr(info, "arguments"):
            if self._event:
                self._event._set_info(info)
        else:
            msg = f"Invalid info type: {type(info)}"
            raise ValueError(msg)

    @classmethod
    def from_info(
        cls, info: active_pb2.PropertyInfo | active_pb2.MethodInfo | active_pb2.EventInfo
    ) -> AxServeMember[T, P, R, Q]:
        instance = cls(info.index)
        if hasattr(info, "property_type"):
            instance._property = AxServeProperty(info.index)
            instance._property._set_info(info)
        elif hasattr(info, "return_type"):
            instance._method = AxServeMethod(info.index)
            instance._method._set_info(info)
        elif hasattr(info, "arguments"):
            instance._event = AxServeEvent(info.index)
            instance._event._set_info(info)
        else:
            msg = f"Invalid info type: {type(info)}"
            raise ValueError(msg)
        return instance

    @overload
    def __get__(
        self, instance: Any, owner: type | None = None
    ) -> Awaitable[T] | AxServeMethodType[P, R] | AxServeEventType[Q] | AxServeMemberType[T, P, R, Q]:
        ...

    @overload
    def __get__(
        self, instance: None, owner: type
    ) -> AxServeMember[T, P, R, Q] | AxServeProperty[T] | AxServeMethod[P, R] | AxServeEvent[Q]:
        ...

    def __get__(
        self,
        instance: AbstractAxServeObject | None = None,
        owner: type | None = None,
    ) -> (
        AxServeMember[T, P, R, Q]
        | AxServeProperty[T]
        | Awaitable[T]
        | AxServeMethod[P, R]
        | AxServeMethodType[P, R]
        | AxServeEvent[Q]
        | AxServeEventType[Q]
        | AxServeMemberType[T, P, R, Q]
    ):
        if instance is None:
            return self
        if self._property and not self._method and not self._event:
            return self._property.__get__(instance, owner)
        if not self._property and self._method and not self._event:
            return self._method.__get__(instance, owner)
        if not self._property and not self._method and self._event:
            return self._event.__get__(instance, owner)
        if (
            self._property
            and self._method
            and not self._event
            and self._method._info
            and self._property._info
            and (
                len(self._method._info.arguments) == 0
                and self._method._info.return_type == self._property._info.property_type
            )
        ):
            return self._property.__get__(instance, owner)
        if self._property or self._method or self._event:
            return AxServeMemberType(self, instance)
        raise NotImplementedError()

    def __set__(
        self,
        instance: AbstractAxServeObject,
        value: T,
    ) -> Task[active_pb2.SetPropertyResponse]:
        if self._property:
            return self._property.__set__(instance, value)
        raise NotImplementedError()

    def __call__(self, instance: AbstractAxServeObject, *args, **kwargs) -> Awaitable[R] | Awaitable[None]:
        if self._method:
            return self._method(instance, *args, **kwargs)
        if self._event:
            return self._event(instance, *args, **kwargs)
        raise NotImplementedError()
