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

import asyncio
import functools
import inspect

from asyncio import Task
from typing import TYPE_CHECKING
from typing import Any
from typing import Concatenate
from typing import Generic
from typing import ParamSpec
from typing import TypeVar
from typing import overload

from wrapt import ObjectProxy

from axserve.aio.common.async_connectable import AsyncConnectable
from axserve.proto import active_pb2
from axserve.proto.active_pb2_conversion import AnnotationFromTypeName
from axserve.proto.active_pb2_conversion import ValueFromVariant
from axserve.proto.active_pb2_conversion import ValueToVariant


if TYPE_CHECKING:
    from collections.abc import Awaitable
    from collections.abc import Callable
    from collections.abc import Sequence

    from axserve.aio.client.stub import AxServeObject

T = TypeVar("T")
U = TypeVar("U")
R = TypeVar("R")

P = ParamSpec("P")
Q = ParamSpec("Q")


class AxServePropertyType(ObjectProxy, Generic[T]):
    def __init__(self, prop: AxServeProperty[T], instance: AxServeObject) -> None:
        super().__init__(prop)
        self._self_prop = prop
        self._self_instance = instance

    def get(self) -> Awaitable[T]:
        return self._self_prop._get_value(self._self_instance)

    def set(self, value: T) -> Awaitable[active_pb2.SetPropertyResponse]:
        return self._self_prop._set(self._self_instance, value)


class AxServeProperty(Generic[T]):
    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, arg: int) -> None: ...

    @overload
    def __init__(self, arg: active_pb2.PropertyInfo) -> None: ...

    @overload
    def __init__(self, arg: Callable[[Any], Awaitable[T]]) -> None: ...

    def __init__(
        self,
        arg: int
        | active_pb2.PropertyInfo
        | Callable[[Any], Awaitable[T]]
        | None = None,
    ) -> None:
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

    def _get_index(self, instance: AxServeObject) -> int:
        index = self._index
        if index is not None:
            return index
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        mm = ax._members_manager
        if mm is None:
            msg = "Members manager is not initialized"
            raise ValueError(msg)
        name = self._name
        if name is None:
            msg = "Cannot specify member name"
            raise ValueError(msg)
        if not mm._has_member_name(name):
            msg = f"Object has no member '{name}'"
            raise ValueError(msg)
        member = mm._get_member_by_name(name)
        prop = member._property
        if prop is None:
            msg = f"Object member '{name}' is not a property"
            raise ValueError(msg)
        index = prop._index
        if index is None:
            msg = "Cannot specify property index"
            raise ValueError(msg)
        return index

    async def _get_value(
        self,
        instance: AxServeObject,
        owner: type | None = None,  # noqa: ARG002
    ) -> T:
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        client = ax._client
        instance_id = ax._instance
        if not (client and instance_id):
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        if (
            self._info is not None
            and not self._info.is_readable
            and self._info.is_writable
        ):
            msg = "This property is not readable"
            raise AttributeError(msg)
        index = self._get_index(instance)
        request = active_pb2.GetPropertyRequest()
        request.instance = instance_id
        request.index = index
        client._event_context_manager._contextualize_request(request)
        response = await client._stub.GetProperty(request)
        return ValueFromVariant(response.value)

    async def _get(
        self,
        instance: AxServeObject | None = None,
        owner: type | None = None,
    ) -> AxServeProperty[T] | T:
        if instance is None:
            return self
        return await self._get_value(instance, owner)

    async def _set(
        self,
        instance: AxServeObject,
        value: T,
    ) -> active_pb2.SetPropertyResponse:
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        client = ax._client
        instance_id = ax._instance
        if not (client and instance_id):
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        if (
            self._info is not None
            and not self._info.is_writable
            and self._info.is_readable
        ):
            msg = "This property is not writable"
            raise AttributeError(msg)
        index = self._get_index(instance)
        request = active_pb2.SetPropertyRequest()
        request.instance = instance_id
        request.index = index
        ValueToVariant(value, request.value)
        client._event_context_manager._contextualize_request(request)
        response = await client._stub.SetProperty(request)
        return response

    def __call__(self, instance: AxServeObject) -> Awaitable[T]:
        return self._get_value(instance)

    @overload
    def __get__(self, instance: Any, owner: type | None = None) -> Awaitable[T]: ...

    @overload
    def __get__(self, instance: None, owner: type) -> AxServeProperty[T]: ...

    def __get__(
        self,
        instance: AxServeObject | None = None,
        owner: type | None = None,
    ) -> AxServeProperty[T] | Awaitable[T]:
        if instance is None:
            return self
        return self._get_value(instance, owner)

    def __set__(
        self,
        instance: AxServeObject,
        value: T,
    ) -> Task[active_pb2.SetPropertyResponse]:
        coro = self._set(instance, value)
        task = asyncio.create_task(coro)
        return task


class AxServeMethodType(ObjectProxy, Generic[P, R]):
    def __init__(self, func: AxServeMethod[P, R], instance: AxServeObject) -> None:
        super().__init__(func)
        self._self_func = func
        self._self_instance = instance

    def __call__(self, *args: P.args, **kwargs: P.kwargs) -> Awaitable[R]:
        return self._self_func(self._self_instance, *args, **kwargs)

    def call(self, *args: P.args, **kwargs: P.kwargs) -> Awaitable[R]:
        return self.__call__(*args, **kwargs)


class AxServeMethod(Generic[P, R]):
    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, arg: int) -> None: ...

    @overload
    def __init__(self, arg: active_pb2.MethodInfo) -> None: ...

    @overload
    def __init__(self, arg: Callable[Concatenate[Any, P], Awaitable[R]]) -> None: ...

    def __init__(
        self,
        arg: int
        | active_pb2.MethodInfo
        | Callable[Concatenate[Any, P], Awaitable[R]]
        | None = None,
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

    def _get_index(self, instance: AxServeObject) -> int:
        index = self._index
        if index is not None:
            return index
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        mm = ax._members_manager
        if mm is None:
            msg = "Members manager is not initialized"
            raise ValueError(msg)
        name = self._name
        if name is None:
            msg = "Cannot specify member name"
            raise ValueError(msg)
        if not mm._has_member_name(name):
            msg = f"Object has no member '{name}'"
            raise ValueError(msg)
        member = mm._get_member_by_name(name)
        method = member._method
        if method is None:
            msg = f"Object member '{name}' is not a method"
            raise ValueError(msg)
        index = method._index
        if index is None:
            msg = "Cannot specify method index"
            raise ValueError(msg)
        return index

    def _bind_args(self, *args, **kwargs) -> Sequence[Any]:
        if not self._signature:
            return args
        bound_args = self._signature.bind(*args, **kwargs)
        bound_args.apply_defaults()
        return bound_args.args

    async def _call(
        self, instance: AxServeObject, *args: P.args, **kwargs: P.kwargs
    ) -> R:
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        client = ax._client
        instance_id = ax._instance
        if not (client and instance_id):
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        index = self._get_index(instance)
        request = active_pb2.InvokeMethodRequest()
        request.instance = instance_id
        request.index = index
        bound_args = self._bind_args(*args, **kwargs)
        for arg in bound_args:
            ValueToVariant(arg, request.arguments.add())
        client._event_context_manager._contextualize_request(request)
        response = await client._stub.InvokeMethod(request)
        return ValueFromVariant(response.return_value)

    def __call__(
        self, instance: AxServeObject, *args: P.args, **kwargs: P.kwargs
    ) -> Awaitable[R]:
        return self._call(instance, *args, **kwargs)

    @overload
    def __get__(
        self, instance: Any, owner: type | None = None
    ) -> AxServeMethodType[P, R]: ...

    @overload
    def __get__(self, instance: None, owner: type) -> AxServeMethod[P, R]: ...

    def __get__(
        self,
        instance: AxServeObject | None = None,
        owner: type | None = None,
    ) -> AxServeMethod[P, R] | AxServeMethodType[P, R]:
        if instance is None:
            return self
        return AxServeMethodType(self, instance)


class AxServeEventType(
    ObjectProxy,
    AsyncConnectable[
        P,
        active_pb2.ConnectEventResponse,
        active_pb2.DisconnectEventResponse,
    ],
    Generic[P],
):
    def __init__(self, func: AxServeEvent[P], instance: AxServeObject) -> None:
        super().__init__(func)
        self._self_func = func
        self._self_instance = instance

    def __call__(self, *args: P.args, **kwargs: P.kwargs) -> Awaitable[None]:
        return self._self_func(self._self_instance, *args, **kwargs)

    def call(self, *args: P.args, **kwargs: P.kwargs) -> Awaitable[None]:
        return self.__call__(*args, **kwargs)

    async def connect(
        self, handler: Callable[P, Any]
    ) -> active_pb2.ConnectEventResponse | None:
        response = None
        instance = self._self_instance
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        client = ax._client
        instance_id = ax._instance
        handlers_manager = ax._event_handlers_manager
        if not (client and instance_id and handlers_manager):
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        index = self._self_func._get_index(instance)
        handlers = handlers_manager._get_event_handlers(index)
        handlers_lock = handlers_manager._get_event_handlers_lock(index)
        async with handlers_lock:
            if not handlers:
                request = active_pb2.ConnectEventRequest()
                request.instance = instance_id
                request.index = index
                client._event_context_manager._contextualize_request(request)
                response = await client._stub.ConnectEvent(request)
                if not response.successful:
                    msg = "Failed to connect event"
                    raise RuntimeError(msg)
            handlers.append(handler)
        return response

    async def disconnect(
        self, handler: Callable[P, Any]
    ) -> active_pb2.DisconnectEventResponse | None:
        response = None
        instance = self._self_instance
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        client = ax._client
        instance_id = ax._instance
        handlers_manager = ax._event_handlers_manager
        if not (client and instance_id and handlers_manager):
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        index = self._self_func._get_index(instance)
        handlers = handlers_manager._get_event_handlers(index)
        handlers_lock = handlers_manager._get_event_handlers_lock(index)
        async with handlers_lock:
            handlers.remove(handler)
            if not handlers:
                request = active_pb2.DisconnectEventRequest()
                request.instance = instance_id
                request.index = index
                client._event_context_manager._contextualize_request(request)
                response = await client._stub.DisconnectEvent(request)
                if not response.successful:
                    msg = "Failed to disconnect event"
                    raise RuntimeError(msg)
        return response


class AxServeEvent(Generic[P]):
    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, arg: int) -> None: ...

    @overload
    def __init__(self, arg: active_pb2.EventInfo) -> None: ...

    @overload
    def __init__(self, arg: Callable[Concatenate[Any, P], Awaitable[Any]]) -> None: ...

    def __init__(
        self,
        arg: int
        | active_pb2.EventInfo
        | Callable[Concatenate[Any, P], Awaitable[Any]]
        | None = None,
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

    def _get_index(self, instance: AxServeObject) -> int:
        index = self._index
        if index is not None:
            return index
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        mm = ax._members_manager
        if mm is None:
            msg = "Members manager is not initialized"
            raise ValueError(msg)
        name = self._name
        if name is None:
            msg = "Cannot specify member name"
            raise ValueError(msg)
        if not mm._has_member_name(name):
            msg = f"Object has no member '{name}'"
            raise ValueError(msg)
        member = mm._get_member_by_name(name)
        event = member._event
        if event is None:
            msg = f"Object member '{name}' is not a event"
            raise ValueError(msg)
        index = event._index
        if index is None:
            msg = "Cannot specify event index"
            raise ValueError(msg)
        return index

    async def _call(
        self, instance: AxServeObject, *args: P.args, **kwargs: P.kwargs
    ) -> None:
        ax = instance.__axserve__
        if ax is None:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        handlers_manager = ax._event_handlers_manager
        if not handlers_manager:
            msg = "Internal values are not initialized"
            raise ValueError(msg)
        index = self._get_index(instance)
        handlers = handlers_manager._get_event_handlers(index)
        handlers_lock = handlers_manager._get_event_handlers_lock(index)
        async with handlers_lock:
            handlers = list(handlers)
        for handler in handlers:
            res = handler(*args, **kwargs)
            if inspect.isawaitable(res):
                await res

    def __call__(
        self, instance: AxServeObject, *args: P.args, **kwargs: P.kwargs
    ) -> Awaitable[None]:
        return self._call(instance, *args, **kwargs)

    @overload
    def __get__(
        self, instance: Any, owner: type | None = None
    ) -> AxServeEventType[P]: ...

    @overload
    def __get__(self, instance: None, owner: type) -> AxServeEvent[P]: ...

    def __get__(
        self,
        instance: AxServeObject | None = None,
        owner: type | None = None,
    ) -> AxServeEvent[P] | AxServeEventType[P]:
        if instance is None:
            return self
        return AxServeEventType(self, instance)


class AxServeMemberType(
    ObjectProxy,
    AsyncConnectable[
        Q,
        active_pb2.ConnectEventResponse,
        active_pb2.DisconnectEventResponse,
    ],
    Generic[T, P, R, Q],
):
    def __init__(
        self, member: AxServeMember[T, P, R, Q], instance: AxServeObject
    ) -> None:
        super().__init__(member)
        self._self_mem = member
        self._self_instance = instance

    @property
    def prop(self) -> AxServePropertyType[T]:
        if self._self_mem._property:
            return AxServePropertyType(self._self_mem._property, self._self_instance)
        raise NotImplementedError

    @property
    def method(self) -> AxServeMethodType[P, R]:
        if self._self_mem._method:
            return self._self_mem._method.__get__(self._self_instance)
        raise NotImplementedError

    @property
    def event(self) -> AxServeEventType[Q]:
        if self._self_mem._event:
            return self._self_mem._event.__get__(self._self_instance)
        raise NotImplementedError

    async def get(self) -> T:
        return await self.prop.get()

    async def set(self, value: T) -> active_pb2.SetPropertyResponse:
        return await self.prop.set(value)

    def __call__(self, *args, **kwargs) -> Awaitable[R] | Awaitable[None]:
        return self._self_mem(self._self_instance, *args, **kwargs)

    async def call(self, *args, **kwargs) -> R | None:
        return await self.__call__(*args, **kwargs)

    async def connect(
        self, handler: Callable[Q, Any]
    ) -> active_pb2.ConnectEventResponse | None:
        return await self.event.connect(handler)

    async def disconnect(
        self, handler: Callable[Q, Any]
    ) -> active_pb2.DisconnectEventResponse | None:
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
                raise TypeError(msg)

        if prop is not None:
            self._property = prop
        if method is not None:
            self._method = method
        if event is not None:
            self._event = event

    def __set_name__(self, owner, name: str) -> None:
        self._name = name

    def _set_info(
        self,
        info: active_pb2.PropertyInfo | active_pb2.MethodInfo | active_pb2.EventInfo,
    ) -> None:
        if isinstance(info, active_pb2.PropertyInfo):
            if self._property:
                self._property._set_info(info)
        elif isinstance(info, active_pb2.MethodInfo):
            if self._method:
                self._method._set_info(info)
        elif isinstance(info, active_pb2.EventInfo):
            if self._event:
                self._event._set_info(info)
        else:
            msg = f"Invalid info type: {type(info)}"
            raise TypeError(msg)

    @classmethod
    def from_info(
        cls,
        info: active_pb2.PropertyInfo | active_pb2.MethodInfo | active_pb2.EventInfo,
    ) -> AxServeMember[T, P, R, Q]:
        instance = cls()
        if isinstance(info, active_pb2.PropertyInfo):
            instance._property = AxServeProperty(info.index)
            instance._property._set_info(info)
        elif isinstance(info, active_pb2.MethodInfo):
            instance._method = AxServeMethod(info.index)
            instance._method._set_info(info)
        elif isinstance(info, active_pb2.EventInfo):
            instance._event = AxServeEvent(info.index)
            instance._event._set_info(info)
        else:
            msg = f"Invalid info type: {type(info)}"
            raise TypeError(msg)
        return instance

    @overload
    def __get__(
        self, instance: Any, owner: type | None = None
    ) -> (
        Awaitable[T]
        | AxServeMethodType[P, R]
        | AxServeEventType[Q]
        | AxServeMemberType[T, P, R, Q]
    ): ...

    @overload
    def __get__(
        self, instance: None, owner: type
    ) -> (
        AxServeMember[T, P, R, Q]
        | AxServeProperty[T]
        | AxServeMethod[P, R]
        | AxServeEvent[Q]
    ): ...

    def __get__(
        self,
        instance: AxServeObject | None = None,
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
        raise NotImplementedError

    def __set__(
        self,
        instance: AxServeObject,
        value: T,
    ) -> Task[active_pb2.SetPropertyResponse]:
        if self._property:
            return self._property.__set__(instance, value)
        raise NotImplementedError

    def __call__(
        self, instance: AxServeObject, *args, **kwargs
    ) -> Awaitable[R] | Awaitable[None]:
        if self._method:
            return self._method(instance, *args, **kwargs)
        if self._event:
            return self._event(instance, *args, **kwargs)
        raise NotImplementedError
