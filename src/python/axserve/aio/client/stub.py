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
import atexit
import contextlib
import inspect
import threading
import typing

from asyncio import Task
from typing import Any
from typing import AsyncIterator
from typing import Awaitable
from typing import Callable
from typing import Iterable
from typing import Optional
from typing import Union

import grpc
import pywintypes

from axserve.aio.common.async_iterable_queue import AsyncIterableQueue
from axserve.aio.server.process import AxServeServerProcess
from axserve.aio.server.process import CreateAxServeServerProcess
from axserve.common.socket import FindFreePort
from axserve.proto import active_pb2
from axserve.proto import active_pb2_grpc
from axserve.proto.variant_conversion import AnnotationFromTypeName
from axserve.proto.variant_conversion import ValueFromVariant
from axserve.proto.variant_conversion import ValueToVariant


class AxServeProperty:
    def __init__(
        self,
        obj: AxServeObject,
        info: active_pb2.PropertyInfo,
    ):
        self._obj = obj
        self._info = info

    def __set_name__(self, owner, name):
        if owner != self._obj:
            raise ValueError("Given owner is different with the object instance")
        if name != self._info.name:
            raise ValueError("Given name is different with the info's name")

    async def _get(
        self,
        obj: Optional[AxServeObject] = None,
        objtype: Optional[type] = None,
    ):
        if obj is None:
            return self
        request = active_pb2.GetPropertyRequest()
        request.index = self._info.index
        obj._set_request_context(request)
        response = await obj._stub.GetProperty(request)
        response = typing.cast(active_pb2.GetPropertyResponse, response)
        return ValueFromVariant(response.value)

    async def _set(
        self,
        obj: AxServeObject,
        value: Any,
    ):
        request = active_pb2.SetPropertyRequest()
        request.index = self._info.index
        ValueToVariant(value, request.value)
        obj._set_request_context(request)
        response = await obj._stub.SetProperty(request)
        response = typing.cast(active_pb2.SetPropertyResponse, response)
        return response

    def get(self):
        return self._get(self._obj)

    def set(self, value):
        return self._set(self._obj, value)

    def __get__(
        self,
        obj: Optional[AxServeObject] = None,
        objtype: Optional[type] = None,
    ):
        return self._get(obj, objtype)

    def __set__(
        self,
        obj: AxServeObject,
        value: Any,
    ):
        coro = self._set(obj, value)
        task = asyncio.create_task(coro)
        return task


class AxServeMethod:
    def __init__(
        self,
        obj: AxServeObject,
        info: active_pb2.MethodInfo,
    ):
        self._obj = obj
        self._info = info
        self._sig = inspect.Signature(
            parameters=[
                inspect.Parameter(
                    name=arg.name,
                    kind=inspect.Parameter.POSITIONAL_OR_KEYWORD,
                    annotation=AnnotationFromTypeName(arg.argument_type),
                )
                for arg in self._info.arguments
            ],
            return_annotation=AnnotationFromTypeName(self._info.return_type),
        )
        self.__name__ = self._info.name
        self.__signature__ = self._sig

    async def call(self, *args, **kwargs):
        request = active_pb2.InvokeMethodRequest()
        request.index = self._info.index
        bound_args = self._sig.bind(*args, **kwargs)
        bound_args.apply_defaults()
        for arg in bound_args.args:
            ValueToVariant(arg, request.arguments.add())
        self._obj._set_request_context(request)
        response = await self._obj._stub.InvokeMethod(request)
        response = typing.cast(active_pb2.InvokeMethodResponse, response)
        return ValueFromVariant(response.return_value)

    def __call__(self, *args, **kwargs):
        return self.call(*args, **kwargs)


class AxServeEvent:
    def __init__(
        self,
        obj: AxServeObject,
        info: active_pb2.EventInfo,
    ):
        self._obj = obj
        self._info = info
        self._sig = inspect.Signature(
            parameters=[
                inspect.Parameter(
                    name=arg.name,
                    kind=inspect.Parameter.POSITIONAL_OR_KEYWORD,
                    annotation=AnnotationFromTypeName(arg.argument_type),
                )
                for arg in self._info.arguments
            ],
            return_annotation=None,
        )
        self._handlers: list[Callable[..., Awaitable | None]] = []
        self._handlers_lock = asyncio.Lock()
        self.__name__ = self._info.name
        self.__signature__ = self._sig

    async def connect(self, handler):
        response = None
        async with self._handlers_lock:
            if not self._handlers:
                request = active_pb2.ConnectEventRequest()
                request.index = self._info.index
                self._obj._set_request_context(request)
                response = await self._obj._stub.ConnectEvent(request)
                response = typing.cast(active_pb2.ConnectEventResponse, response)
                if not response.successful:
                    raise RuntimeError("Failed to connect event")
            self._handlers.append(handler)
        return response

    async def disconnect(self, handler):
        response = None
        async with self._handlers_lock:
            self._handlers.remove(handler)
            if not self._handlers:
                request = active_pb2.DisconnectEventRequest()
                request.index = self._info.index
                self._obj._set_request_context(request)
                response = await self._obj._stub.DisconnectEvent(request)
                response = typing.cast(active_pb2.DisconnectEventResponse, response)
                if not response.successful:
                    raise RuntimeError("Failsed to disconnect event")
        return response

    async def call(self, *args, **kwargs):
        async with self._handlers_lock:
            handlers = list(self._handlers)
        for handler in handlers:
            res = handler(*args, **kwargs)
            if inspect.isawaitable(res):
                await res

    def __call__(self, *args, **kwargs):
        return self.call(*args, **kwargs)


class AxServeMember:
    def __init__(self, obj: AxServeObject):
        self._obj = obj
        self._prop: Optional[AxServeProperty] = None
        self._method: Optional[AxServeMethod] = None
        self._event: Optional[AxServeEvent] = None

    @property
    def prop(self):
        return self._prop

    @property
    def method(self):
        return self._method

    @property
    def event(self):
        return self._event

    def get(self):
        if self._prop:
            return self._prop._get(self._obj)
        raise NotImplementedError()

    def set(self, value):
        if self._prop:
            return self._prop._set(self._obj, value)
        raise NotImplementedError()

    def __get__(
        self,
        obj: Optional[AxServeObject] = None,
        objtype: Optional[type] = None,
    ):
        if self._prop:
            return self._prop.__get__(obj, objtype)
        if self._method:
            return self._method
        if self._event:
            return self._event
        raise NotImplementedError()

    def __set__(
        self,
        obj: AxServeObject,
        value: Any,
    ):
        if self._prop:
            return self._prop.__set__(obj, value)
        raise NotImplementedError()

    def call(self, *args, **kwargs):
        if self._method:
            return self._method.call(*args, **kwargs)
        if self._event:
            return self._event.call(*args, **kwargs)
        raise NotImplementedError()

    def __call__(self, *args, **kwargs):
        return self.call(*args, **kwargs)

    def connect(self, handler):
        if self._event:
            return self._event.connect(handler)
        raise NotImplementedError()

    def disconnect(self, handler):
        if self._event:
            return self._event.disconnect(handler)
        raise NotImplementedError()


class AxServeEventLoop:
    def __init__(self, obj: AxServeObject):
        self._obj = obj
        self._return_code = 0
        self._state_lock = asyncio.Lock()
        self._is_exitting = False
        self._is_running = False

    @contextlib.asynccontextmanager
    async def _create_exec_context(self):
        async with self._state_lock:
            self._is_exitting = False
            self._is_running = True
        try:
            yield
        finally:
            async with self._state_lock:
                self._is_exitting = False
                self._is_running = False

    @contextlib.asynccontextmanager
    async def _create_handle_event_context(self, handle_event: active_pb2.HandleEventRequest):
        event_context_stack = self._obj._get_handle_event_context_stack()
        event_context_stack.append(handle_event)
        try:
            yield
        finally:
            event_context_stack = self._obj._get_handle_event_context_stack()
            event_context_stack.pop()
            response = active_pb2.HandleEventResponse()
            response.index = handle_event.index
            response.id = handle_event.id
            if self._obj._handle_event_requests:
                handle_events = typing.cast(grpc.aio.StreamStreamCall, self._obj._handle_event_requests)
                await handle_events.write(response)

    async def exec(self) -> int:
        async with self._create_exec_context():
            if not self._obj._handle_event_requests:
                self._obj._handle_event_requests = self._obj._stub.HandleEvent()

            handle_events = typing.cast(
                AsyncIterator[active_pb2.HandleEventRequest] | grpc.aio.StreamStreamCall,
                self._obj._handle_event_requests,
            )

            try:
                async for handle_event in handle_events:
                    async with self._create_handle_event_context(handle_event):
                        args = [ValueFromVariant(arg) for arg in handle_event.arguments]
                        await self._obj._events_list[handle_event.index](*args)
            except grpc.RpcError as exc:
                if not (
                    self._is_exitting
                    and isinstance(exc, grpc.aio.AioRpcError)
                    and exc.code() == grpc.StatusCode.CANCELLED
                ):
                    raise exc

        return self._return_code

    def is_running(self) -> bool:
        return self._is_running

    async def exit(self, return_code: int = 0) -> None:
        async with self._state_lock:
            if not self._is_running:
                return
            if self._is_exitting:
                return
            self._is_exitting = True
            self._return_code = return_code
        if self._obj._handle_event_requests:
            handle_events = typing.cast(grpc.aio.StreamStreamCall, self._obj._handle_event_requests)
            await handle_events.done_writing()


AxServeCommonRequest = Union[
    active_pb2.DescribeRequest,
    active_pb2.GetPropertyRequest,
    active_pb2.SetPropertyRequest,
    active_pb2.InvokeMethodRequest,
    active_pb2.ConnectEventRequest,
    active_pb2.DisconnectEventRequest,
]


class AxServeObject:
    async def __ainit__(
        self,
        channel: Union[grpc.aio.Channel, str],
        *,
        start_event_loop: Optional[bool] = True,
    ):
        try:
            if isinstance(channel, str):
                try:
                    pywintypes.IID(channel)
                except pywintypes.com_error:
                    address = channel
                    channel = grpc.aio.insecure_channel(address)
                    self._channel = channel
                else:
                    clsid = channel
                    port = FindFreePort()
                    address = f"localhost:{port}"
                    server_process = await CreateAxServeServerProcess(clsid, address)
                    channel = grpc.aio.insecure_channel(address)
                    self._server_process = server_process
                    self._channel = channel

            if start_event_loop is None:
                start_event_loop = True

            await channel.channel_ready()

            self._stub = active_pb2_grpc.ActiveStub(channel)
            request = active_pb2.DescribeRequest()
            self._set_request_context(request)
            response = await self._stub.Describe(request)
            response = typing.cast(active_pb2.DescribeResponse, response)

            self._members_dict: dict[str, AxServeMember] = {}

            self._properties_list: list[AxServeProperty] = []
            self._properties_dict: dict[str, AxServeProperty] = {}
            self._methods_list: list[AxServeMethod] = []
            self._methods_dict: dict[str, AxServeMethod] = {}
            self._events_list: list[AxServeEvent] = []
            self._events_dict: dict[str, AxServeEvent] = {}

            for info in response.properties:
                prop = AxServeProperty(self, info)
                self._properties_list.append(prop)
                self._properties_dict[info.name] = prop
                if info.name not in self._members_dict:
                    self._members_dict[info.name] = AxServeMember(self)
                self._members_dict[info.name]._prop = prop
            for info in response.methods:
                method = AxServeMethod(self, info)
                self._methods_list.append(method)
                self._methods_dict[info.name] = method
                if info.name not in self._members_dict:
                    self._members_dict[info.name] = AxServeMember(self)
                self._members_dict[info.name]._method = method
            for info in response.events:
                event = AxServeEvent(self, info)
                self._events_list.append(event)
                self._events_dict[info.name] = event
                if info.name not in self._members_dict:
                    self._members_dict[info.name] = AxServeMember(self)
                self._members_dict[info.name]._event = event

            self._handle_event_requests = self._stub.HandleEvent()
            self._event_loop = AxServeEventLoop(self)

            if start_event_loop:
                self._event_loop_exec_coro = self._event_loop_exec_target()
                self._event_loop_exec_task = asyncio.create_task(self._event_loop_exec_coro)

        except Exception:
            await self.close()
            raise
        else:
            atexit.register(self._close_sync)

    def __init__(
        self,
        channel: Union[grpc.aio.Channel, str],
        *,
        start_event_loop: Optional[bool] = True,
    ):
        self.__dict__["_members_dict"] = {}
        self.__dict__["_properties_dict"] = {}

        self._thread_local = threading.local()
        self._thread_local._handle_event_context_stack = []

        self._handle_event_response_queue: Optional[AsyncIterableQueue] = None
        self._handle_event_requests: Optional[AsyncIterator[active_pb2.HandleEventRequest]] = None

        self._event_loop: Optional[AxServeEventLoop] = None
        self._event_loop_exception: Optional[Exception] = None

        self._server_process: Optional[AxServeServerProcess] = None
        self._channel: Optional[grpc.aio.Channel] = None

        self._init_task: Optional[Task] = None
        self._event_loop_exec_task: Optional[Task] = None

        self._init_coro = self.__ainit__(channel, start_event_loop=start_event_loop)
        self._init_task = asyncio.create_task(self._init_coro)

    def __getattr__(self, name):
        if name in self._members_dict:
            return self._members_dict[name].__get__(self)
        return super().__getattribute__(name)

    def __setattr__(self, name, value):
        if name in self._properties_dict:
            return self._properties_dict[name].__set__(self, value)
        return super().__setattr__(name, value)

    def __getitem__(self, name) -> AxServeMember:
        if name in self._members_dict:
            return self._members_dict[name]
        raise KeyError(name)

    def __dir__(self) -> Iterable[str]:
        members = list(self._members_dict.keys())
        attrs = list(super().__dir__())
        return members + attrs

    def _get_handle_event_context_stack(self) -> list[active_pb2.HandleEventRequest]:
        if not hasattr(self._thread_local, "_handle_event_context_stack"):
            self._thread_local._handle_event_context_stack = []
        return self._thread_local._handle_event_context_stack

    def _set_request_context(self, request: AxServeCommonRequest) -> AxServeCommonRequest:
        event_context_stack = self._get_handle_event_context_stack()
        if event_context_stack:
            callback_event_index = event_context_stack[-1].index
            request.request_context = active_pb2.RequestContext.EVENT_CALLBACK
            request.callback_event_index = callback_event_index
        return request

    async def _event_loop_exec_target(self):
        try:
            await self._event_loop.exec()
        except grpc.RpcError as exc:
            self._event_loop_exception = exc
            self = None
            if (
                isinstance(exc, grpc.aio.AioRpcError)
                and exc.code() == grpc.StatusCode.CANCELLED
                and isinstance(exc, grpc.aio.RpcContext)
                and exc.done()
            ):
                return
            raise exc
        except Exception as exc:
            self._event_loop_exception = exc
            self = None
            raise exc

    @property
    def event_loop(self) -> AxServeEventLoop:
        if self._event_loop is None:
            raise ValueError("Member event_loop is not initialized")
        return self._event_loop

    async def close(self):
        if self._event_loop:
            await self._event_loop.exit()
        if self._event_loop_exec_task:
            await self._event_loop_exec_task
        if self._channel:
            await self._channel.close()
        if self._server_process:
            self._server_process.terminate()
            await self._server_process.wait()

    def _close_sync(self):
        loop = asyncio.get_event_loop()
        loop.run_until_complete(self.close())

    async def __aenter__(self):
        await self._init_task
        return self

    async def __aexit__(self, exc_type, exc_value, traceback):
        await self.close()
