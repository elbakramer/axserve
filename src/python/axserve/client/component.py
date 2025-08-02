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

import contextlib
import threading
import typing

from collections import defaultdict
from collections.abc import Callable
from collections.abc import Iterator
from collections.abc import Mapping
from threading import Condition
from threading import Thread
from typing import TYPE_CHECKING
from typing import TypeVar
from weakref import WeakValueDictionary

import grpc

from axserve.client.descriptor import AxServeEvent
from axserve.client.descriptor import AxServeMember
from axserve.client.descriptor import AxServeMethod
from axserve.client.descriptor import AxServeProperty
from axserve.common.acquireable import Acquireable
from axserve.common.iterable_queue import IterableQueue
from axserve.proto import active_pb2
from axserve.proto.active_pb2_conversion import ValueFromVariant
from axserve.proto.active_pb2_grpc import ActiveStub


if TYPE_CHECKING:
    from axserve.client.stub import AxServeObject


T = TypeVar("T")


class AxServeEventContextManager:
    def __init__(self):
        self._thread_local = threading.local()
        self._thread_local._handle_event_context_stack = []

    def _get_handle_event_context_stack(
        self,
    ) -> list[active_pb2.HandleEventRequest]:
        if not hasattr(self._thread_local, "_handle_event_context_stack"):
            self._thread_local._handle_event_context_stack = []
        return self._thread_local._handle_event_context_stack

    def _get_current_handle_event(self) -> active_pb2.HandleEventRequest | None:
        handle_event_context_stack = self._get_handle_event_context_stack()
        return handle_event_context_stack[-1] if handle_event_context_stack else None

    def _contextualize_request(self, request):
        current_handle_event = self._get_current_handle_event()
        if not current_handle_event:
            return request
        context_type = active_pb2.ContextType.EVENT
        request.context.context_type = context_type
        request.context.context_info.id = current_handle_event.id
        request.context.context_info.instance = current_handle_event.instance
        request.context.context_info.index = current_handle_event.index
        return request


class AxServeEventHandlersManager:
    def __init__(self):
        self._event_handlers_mapping: Mapping[int, list[Callable]] = defaultdict(list)
        self._event_handlers_lock_mapping: Mapping[int, Acquireable] = defaultdict(
            threading.RLock
        )  # type: ignore

    def _get_event_handlers(self, index: int) -> list[Callable]:
        return self._event_handlers_mapping[index]

    def _get_event_handlers_lock(self, index: int) -> Acquireable:
        return self._event_handlers_lock_mapping[index]


class AxServeEventStreamManager:
    def __init__(self, stub: ActiveStub):
        self._handle_event_response_queue = IterableQueue()
        self._handle_event_requests = stub.HandleEvent(
            self._handle_event_response_queue
        )

    def _get_handle_event_requests(self) -> Iterator[active_pb2.HandleEventRequest]:
        return self._handle_event_requests

    def _put_handle_event_response(
        self, response: active_pb2.HandleEventResponse
    ) -> None:
        return self._handle_event_response_queue.put(response)

    def _close_event_stream(self) -> None:
        return self._handle_event_response_queue.close()

    def _cancel_event_stream(self) -> bool:
        handle_events = typing.cast(grpc.RpcContext, self._handle_event_requests)
        return handle_events.cancel()


class AxServeInstancesManager:
    _instances: WeakValueDictionary[str, AxServeObject]

    def __init__(self):
        self._instances = WeakValueDictionary()

    def _register_instance(self, i: str, o: AxServeObject):
        self._instances[i] = o

    def _unregister_instance(
        self, i: str, default: T | None = None
    ) -> AxServeObject | T | None:
        return self._instances.pop(i, default)

    def _has_instance(self, i: str) -> bool:
        return i in self._instances

    def _get_instance(
        self, i: str, default: T | None = None
    ) -> AxServeObject | T | None:
        return self._instances.get(i, default)


class AxServeMembersManager:
    _instance: str
    _stub: ActiveStub
    _context_manager: AxServeEventContextManager

    _members_dict: dict[str, AxServeMember]
    _properties_list: list[AxServeProperty]
    _properties_dict: dict[str, AxServeProperty]
    _methods_list: list[AxServeMethod]
    _methods_dict: dict[str, AxServeMethod]
    _events_list: list[AxServeEvent]
    _events_dict: dict[str, AxServeEvent]

    def __init__(
        self,
        instance: str,
        stub: ActiveStub,
        context_manager: AxServeEventContextManager,
    ):
        self._instance = instance
        self._stub = stub
        self._context_manager = context_manager

        self._members_dict = {}
        self._properties_list = []
        self._properties_dict = {}
        self._methods_list = []
        self._methods_dict = {}
        self._events_list = []
        self._events_dict = {}

        request = active_pb2.DescribeRequest()
        request.instance = instance
        context_manager._contextualize_request(request)
        response = stub.Describe(request)
        response = typing.cast(active_pb2.DescribeResponse, response)

        for info in response.properties:
            prop = AxServeProperty(info)
            self._properties_list.append(prop)
            self._properties_dict[info.name] = prop
            if info.name not in self._members_dict:
                self._members_dict[info.name] = AxServeMember()
            self._members_dict[info.name]._property = prop
        for info in response.methods:
            method = AxServeMethod(info)
            self._methods_list.append(method)
            self._methods_dict[info.name] = method
            if info.name not in self._members_dict:
                self._members_dict[info.name] = AxServeMember()
            self._members_dict[info.name]._method = method
        for info in response.events:
            info.name = self._make_event_method_name(info.name)
            event = AxServeEvent(info)
            self._events_list.append(event)
            self._events_dict[info.name] = event
            if info.name not in self._members_dict:
                self._members_dict[info.name] = AxServeMember()
            self._members_dict[info.name]._event = event

    @classmethod
    def _make_event_method_name(cls, name: str) -> str:
        if not name.startswith("On"):
            name = "On" + name
        return name

    def _get_property(self, index: int) -> AxServeProperty:
        return self._properties_list[index]

    def _get_method(self, index: int) -> AxServeMethod:
        return self._methods_list[index]

    def _get_event(self, index: int) -> AxServeEvent:
        return self._events_list[index]

    def _has_member_name(self, name: str) -> bool:
        return name in self._members_dict

    def _get_member_by_name(self, name: str) -> AxServeMember:
        return self._members_dict[name]

    def _get_member_names(self) -> list[str]:
        return list(self._members_dict.keys())


class AxServeMembersManagerCache:
    def __init__(
        self,
        stub: ActiveStub,
        context_manager: AxServeEventContextManager,
    ):
        self._stub = stub
        self._context_manager = context_manager
        self._members_managers = {}

    def _get_members_manager(self, c: str, i: str) -> AxServeMembersManager:
        if c not in self._members_managers:
            members_manager = AxServeMembersManager(
                i, self._stub, self._context_manager
            )
            self._members_managers[c] = members_manager
        members_manager = self._members_managers[c]
        return members_manager


class AxServeEventLoop:
    def __init__(
        self,
        instances_manager: AxServeInstancesManager,
        event_context_manager: AxServeEventContextManager,
        event_stream_manager: AxServeEventStreamManager,
    ):
        self._instances_manager = instances_manager
        self._event_context_manager = event_context_manager
        self._event_stream_manager = event_stream_manager

        self._state_lock = threading.RLock()
        self._is_exitting = False
        self._is_running = False
        self._return_code = 0

    @contextlib.contextmanager
    def _create_exec_context(self):
        with self._state_lock:
            self._is_exitting = False
            self._is_running = True
        try:
            yield
        finally:
            with self._state_lock:
                self._is_exitting = False
                self._is_running = False

    @contextlib.contextmanager
    def _create_handle_event_context(self, handle_event: active_pb2.HandleEventRequest):
        event_context_stack = (
            self._event_context_manager._get_handle_event_context_stack()
        )
        event_context_stack.append(handle_event)
        try:
            yield
        finally:
            event_context_stack.pop()
            response = active_pb2.HandleEventResponse()
            response.id = handle_event.id
            response.instance = handle_event.instance
            response.index = handle_event.index
            self._event_stream_manager._put_handle_event_response(response)

    def exec(self) -> int:
        with self._create_exec_context():
            handle_events = self._event_stream_manager._get_handle_event_requests()
            try:
                for handle_event in handle_events:
                    if handle_event.is_pong:
                        return self._return_code
                    with self._create_handle_event_context(handle_event):
                        instance_id = handle_event.instance
                        instance = self._instances_manager._get_instance(instance_id)
                        if instance is None:
                            continue
                        ax = instance.__axserve__
                        if ax is None:
                            continue
                        mm = ax._members_manager
                        if mm is None:
                            continue
                        event_callback = mm._get_event(handle_event.index)
                        args = [ValueFromVariant(arg) for arg in handle_event.arguments]
                        event_callback(instance, *args)
            except grpc.RpcError as exc:
                if not (
                    self._is_exitting
                    and isinstance(exc, grpc.Call)
                    and exc.code() == grpc.StatusCode.CANCELLED
                ):
                    raise exc
        return self._return_code

    def is_running(self) -> bool:
        return self._is_running

    def wake_up(self) -> None:
        handle_events = self._event_stream_manager._get_handle_event_requests()
        state = getattr(handle_events, "_state", None)
        condition = getattr(state, "condition", None)
        condition = typing.cast(Condition | None, condition)
        if condition is not None:
            with condition:
                condition.notify_all()

    def exit(self, return_code: int = 0) -> None:
        with self._state_lock:
            if not self._is_running:
                return
            if self._is_exitting:
                return
            self._is_exitting = True
            self._return_code = return_code
        response = active_pb2.HandleEventResponse()
        response.is_ping = True
        self._event_stream_manager._put_handle_event_response(response)


class AxServeEventLoopManager:
    def __init__(
        self,
        instances_manager: AxServeInstancesManager,
        event_context_manager: AxServeEventContextManager,
        event_stream_manager: AxServeEventStreamManager,
    ):
        self._instances_manager = instances_manager
        self._event_context_manager = event_context_manager
        self._event_stream_manager = event_stream_manager

        self._event_loop: AxServeEventLoop | None = None
        self._event_loop_thread: Thread | None = None
        self._event_loop_exception: BaseException | None = None

    def _event_loop_exec_target(self) -> None:
        if not self._event_loop:
            return
        try:
            self._event_loop.exec()
        except grpc.RpcError as exc:
            self._event_loop_exception = exc
            self = None  # noqa: PLW0642
            if (
                isinstance(exc, grpc.Call)
                and exc.code() == grpc.StatusCode.CANCELLED
                and isinstance(exc, grpc.RpcContext)
                and not exc.is_active()
            ):
                return
            raise exc
        except BaseException as exc:
            self._event_loop_exception = exc
            self = None  # noqa: PLW0642
            raise exc

    def start(self) -> None:
        if not self._event_loop:
            self._event_loop = AxServeEventLoop(
                self._instances_manager,
                self._event_context_manager,
                self._event_stream_manager,
            )
        if not self._event_loop_thread:
            self._event_loop_thread = Thread(
                target=self._event_loop_exec_target, daemon=True
            )
            self._event_loop_thread.start()

    def is_running(self) -> bool:
        return self._event_loop.is_running() if self._event_loop is not None else False

    def stop(self) -> None:
        if self._event_loop:
            self._event_loop.exit()
            self._event_loop = None
        if self._event_loop_thread:
            self._event_loop_thread.join()
            self._event_loop_thread = None
