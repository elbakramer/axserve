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

from collections.abc import Callable
from collections.abc import Iterable
from collections.abc import Iterator

import grpc

from axserve.client.abstract import AbstractAxServeObject
from axserve.client.component import AxServeEventContextManager
from axserve.client.component import AxServeEventHandlersManager
from axserve.client.component import AxServeEventLoopManager
from axserve.client.component import AxServeEventStreamManager
from axserve.client.component import AxServeMembersManager
from axserve.client.component import AxServeStubManager
from axserve.client.descriptor import AxServeEvent
from axserve.client.descriptor import AxServeMember
from axserve.client.descriptor import AxServeMemberType
from axserve.client.descriptor import AxServeMethod
from axserve.client.descriptor import AxServeProperty
from axserve.common.aquireable import Aquireable
from axserve.proto import active_pb2
from axserve.proto import active_pb2_grpc


class AxServeObject(AbstractAxServeObject):
    def __init__(
        self,
        channel: grpc.Channel | str | int | None = None,
        *,
        channel_ready_timeout: float | None = None,
    ):
        self.__dict__["_members_manager"] = None

        if channel is None and hasattr(self, "__CLSID__"):
            channel = self.__CLSID__

        if channel is None:
            msg = "Cannot specify channel"
            raise ValueError(msg)

        if channel_ready_timeout is None:
            channel_ready_timeout = 10.0

        self._stub_manager: AxServeStubManager | None = None
        self._event_context_manager: AxServeEventContextManager | None = None
        self._event_handlers_manager: AxServeEventHandlersManager | None = None
        self._event_stream_manager: AxServeEventStreamManager | None = None
        self._members_manager: AxServeMembersManager | None = None
        self._event_loop_manager: AxServeEventLoopManager | None = None

        self._stub_manager = AxServeStubManager(channel)
        self._stub_manager._wait_until_channel_ready(channel_ready_timeout)

        self._event_context_manager = AxServeEventContextManager()
        self._event_handlers_manager = AxServeEventHandlersManager()

        stub = self._stub_manager._get_stub()
        current_handle_event = self._event_context_manager._get_current_handle_event()

        self._event_stream_manager = AxServeEventStreamManager(stub)
        self._members_manager = AxServeMembersManager(stub, current_handle_event)

        self._event_loop_manager = AxServeEventLoopManager(self)
        self._event_loop_manager.start()

    def _get_stub(self) -> active_pb2_grpc.ActiveStub:
        return self._stub_manager._get_stub()

    def _get_property(self, index: int) -> AxServeProperty:
        return self._members_manager._get_property(index)

    def _get_method(self, index: int) -> AxServeMethod:
        return self._members_manager._get_method(index)

    def _get_event(self, index: int) -> AxServeEvent:
        return self._members_manager._get_event(index)

    def _has_member_name(self, name: str) -> bool:
        return self._members_manager._has_member_name(name)

    def _get_member_by_name(self, name: str) -> AxServeMember:
        return self._members_manager._get_member_by_name(name)

    def _get_member_names(self) -> list[str]:
        return self._members_manager._get_member_names()

    def _get_current_handle_event(self) -> active_pb2.HandleEventRequest | None:
        return self._event_context_manager._get_current_handle_event()

    def _get_handle_event_context_stack(self) -> list[active_pb2.HandleEventRequest]:
        return self._event_context_manager._get_handle_event_context_stack()

    def _get_event_handlers(self, index: int) -> list[Callable]:
        return self._event_handlers_manager._get_event_handlers(index)

    def _get_event_handlers_lock(self, index: int) -> Aquireable:
        return self._event_handlers_manager._get_event_handlers_lock(index)

    def _get_handle_event_requests(self) -> Iterator[active_pb2.HandleEventRequest]:
        return self._event_stream_manager._get_handle_event_requests()

    def _put_handle_event_response(self, response: active_pb2.HandleEventResponse) -> None:
        return self._event_stream_manager._put_handle_event_response(response)

    def _close_event_stream(self) -> None:
        return self._event_stream_manager._close_event_stream()

    def _cancel_event_stream(self) -> None:
        return self._event_stream_manager._cancel_event_stream()

    def __getattr__(self, name):
        if self._members_manager and self._members_manager._has_member_name(name):
            return self._members_manager._get_member_by_name(name).__get__(self)
        return super().__getattribute__(name)

    def __setattr__(self, name, value):
        if self._members_manager and self._members_manager._has_member_name(name):
            return self._members_manager._get_member_by_name(name).__set__(self, value)
        return super().__setattr__(name, value)

    def __getitem__(self, name) -> AxServeMemberType:
        if self._members_manager and self._members_manager._has_member_name(name):
            member = self._members_manager._get_member_by_name(name)
            return AxServeMemberType(member, self)
        raise KeyError(name)

    def __dir__(self) -> Iterable[str]:
        attrs = super().__dir__()
        attrs = set(attrs)
        if self._members_manager:
            members = self._members_manager._get_member_names()
            attrs = set(attrs) | set(members)
        attrs = list(attrs)
        return attrs

    def close(self):
        if self._event_loop_manager:
            self._event_loop_manager.stop()
        if self._event_stream_manager:
            self._event_stream_manager._close_event_stream()
        if self._stub_manager:
            self._stub_manager._close_managed_channel()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
