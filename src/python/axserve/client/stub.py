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

import platform
import typing

from collections.abc import Iterable
from collections.abc import MutableMapping
from threading import RLock
from types import TracebackType
from typing import ClassVar

import grpc

from grpc import Channel

from axserve.client.component import AxServeEventContextManager
from axserve.client.component import AxServeEventHandlersManager
from axserve.client.component import AxServeEventLoopManager
from axserve.client.component import AxServeEventStreamManager
from axserve.client.component import AxServeInstancesManager
from axserve.client.component import AxServeMembersManager
from axserve.client.component import AxServeMembersManagerCache
from axserve.client.descriptor import AxServeMemberType
from axserve.common.registry import CheckMachineFromCLSID
from axserve.common.socket import FindFreePort
from axserve.proto import active_pb2
from axserve.proto.active_pb2_grpc import ActiveStub
from axserve.server.process import AxServeServerProcess


class AxServeObjectInternals:
    _instance: str
    _clsid: str
    _client: AxServeClient
    _members_manager: AxServeMembersManager
    _event_handlers_manager: AxServeEventHandlersManager


class AxServeClient:
    _clients: ClassVar[MutableMapping[str, AxServeClient]] = {}
    _clients_lock: ClassVar[RLock] = RLock()

    _stub: ActiveStub
    _instances_manager: AxServeInstancesManager
    _event_context_manager: AxServeEventContextManager
    _event_stream_manager: AxServeEventStreamManager
    _event_loop_manager: AxServeEventLoopManager
    _members_managers: AxServeMembersManagerCache

    _managed_channel: Channel | None = None
    _managed_process: AxServeServerProcess | None = None

    @classmethod
    def instance(cls, machine: str | None = None):
        if not machine:
            machine = platform.machine()
        if machine not in cls._clients:
            with cls._clients_lock:
                if machine not in cls._clients:
                    port = FindFreePort()
                    address = f"localhost:{port}"
                    process = AxServeServerProcess(address, machine=machine)
                    channel = grpc.insecure_channel(address)
                    client = AxServeClient(channel)
                    client._managed_channel = channel
                    client._managed_process = process
                    cls._clients[machine] = client
        client = cls._clients[machine]
        return client

    def __init__(
        self,
        channel: Channel,
        timeout: float | None = None,
    ) -> None:
        if not timeout:
            timeout = 15

        self._stub = ActiveStub(channel)
        self._instances_manager = AxServeInstancesManager()

        grpc.channel_ready_future(channel).result(timeout=timeout)

        self._event_context_manager = AxServeEventContextManager()
        self._event_stream_manager = AxServeEventStreamManager(self._stub)
        self._event_loop_manager = AxServeEventLoopManager(
            self._instances_manager,
            self._event_context_manager,
            self._event_stream_manager,
        )
        self._members_managers = AxServeMembersManagerCache(
            self._stub,
            self._event_context_manager,
        )

        self._event_loop_manager.start()

    def _create_instance(self, c: str) -> str:
        request = active_pb2.CreateRequest()
        request.clsid = c
        response = self._stub.Create(request)
        response = typing.cast(active_pb2.CreateResponse, response)
        instance = response.instance
        return instance

    def _destroy_instance(self, i: str) -> bool:
        request = active_pb2.DestroyRequest()
        request.instance = i
        response = self._stub.Destroy(request)
        response = typing.cast(active_pb2.DestroyResponse, response)
        return response.successful

    def _create_internals(self, c: str) -> AxServeObjectInternals:
        i = self._create_instance(c)
        members_manager = self._members_managers._get_members_manager(c, i)
        event_handlers_manager = AxServeEventHandlersManager()
        internals = AxServeObjectInternals()
        internals._instance = i
        internals._clsid = c
        internals._client = self
        internals._members_manager = members_manager
        internals._event_handlers_manager = event_handlers_manager
        return internals

    def _initialize_internals(self, o: AxServeObject, c: str) -> None:
        i = self._create_internals(c)
        o.__dict__["__axserve__"] = i
        self._instances_manager._register_instance(i._instance, o)

    def create(self, c: str) -> AxServeObject:
        o = AxServeObject(c, self)
        return o

    def destroy(self, o: AxServeObject) -> None:
        if not o.__axserve__:
            return
        instance = o.__axserve__._instance
        if not self._instances_manager._has_instance(instance):
            return
        if not self._destroy_instance(instance):
            msg = "Failed to destroy the axserve object"
            raise RuntimeError(msg)
        self._instances_manager._unregister_instance(instance)

    def close(self, timeout: float | None = None) -> None:
        if self._event_loop_manager:
            self._event_loop_manager.stop()
        if self._event_stream_manager:
            self._event_stream_manager._close_event_stream()
        if self._managed_channel:
            self._managed_channel.close()
        if self._managed_process:
            self._managed_process.terminate()
            self._managed_process.wait(timeout=timeout)

    def __enter__(self):
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        exc_traceback: TracebackType | None,
    ) -> None:
        self.close()


class AxServeObject:
    __axserve__: AxServeObjectInternals | None = None

    def __init__(
        self,
        c: str | None = None,
        client: AxServeClient | None = None,
    ) -> None:
        if not c and hasattr(self, "__CLSID__"):
            c = self.__CLSID__
        if not c:
            return
        if not client:
            machine = CheckMachineFromCLSID(c)
            client = AxServeClient.instance(machine)
        client._initialize_internals(self, c)

    def __getitem__(self, name) -> AxServeMemberType:
        if (
            self.__axserve__
            and self.__axserve__._members_manager
            and self.__axserve__._members_manager._has_member_name(name)
        ):
            member = self.__axserve__._members_manager._get_member_by_name(name)
            return AxServeMemberType(member, self)
        raise KeyError(name)

    def __getattr__(self, name):
        if (
            self.__axserve__
            and self.__axserve__._members_manager
            and self.__axserve__._members_manager._has_member_name(name)
        ):
            return self.__axserve__._members_manager._get_member_by_name(name).__get__(self)
        return super().__getattribute__(name)

    def __setattr__(self, name, value):  # type: ignore
        if (
            self.__axserve__
            and self.__axserve__._members_manager
            and self.__axserve__._members_manager._has_member_name(name)
        ):
            return self.__axserve__._members_manager._get_member_by_name(name).__set__(self, value)
        return super().__setattr__(name, value)

    def __dir__(self) -> Iterable[str]:
        if self.__axserve__ and self.__axserve__._members_manager:
            members = self.__axserve__._members_manager._get_member_names()
            attrs = super().__dir__()
            attrs = set(attrs) | set(members)
            attrs = list(attrs)
            return attrs
        return super().__dir__()

    def __enter__(self):
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        exc_traceback: TracebackType | None,
    ) -> None:
        if not self.__axserve__:
            return
        self.__axserve__._client._destroy_instance(self.__axserve__._instance)
