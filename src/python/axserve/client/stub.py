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
import time
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
    _clsid: str | None = None
    _client: AxServeClient | None = None
    _instance: str | None = None
    _members_manager: AxServeMembersManager | None = None
    _event_handlers_manager: AxServeEventHandlersManager | None = None

    def __init__(
        self,
        c: str | None = None,
        *,
        client: AxServeClient | None = None,
    ) -> None:
        self._clsid = c
        self._client = client

    @property
    def clsid(self) -> str:
        if self._clsid is None:
            msg = "CLSID is not set"
            raise ValueError(msg)
        return self._clsid

    @property
    def client(self) -> AxServeClient:
        if self._client is None:
            msg = "Client is not set"
            raise ValueError(msg)
        return self._client

    @property
    def instance(self) -> str:
        if self._instance is None:
            msg = "Instance is not set"
            raise ValueError(msg)
        return self._instance

    @property
    def members_manager(self) -> AxServeMembersManager:
        if self._members_manager is None:
            msg = "Members manager is not set"
            raise ValueError(msg)
        return self._members_manager

    @property
    def event_handlers_manager(self) -> AxServeEventHandlersManager:
        if self._event_handlers_manager is None:
            msg = "Event handlers manager is not set"
            raise ValueError(msg)
        return self._event_handlers_manager


class AxServeClientStore:
    _clients: MutableMapping[str, AxServeClient]
    _clients_lock: RLock

    def __init__(self):
        self._clients = {}
        self._clients_lock = RLock()

    def instance(self, machine: str | None = None):
        if not machine:
            machine = platform.machine()
        if machine not in self._clients:
            with self._clients_lock:
                if machine not in self._clients:
                    port = FindFreePort()
                    address = f"localhost:{port}"
                    process = AxServeServerProcess(address, machine=machine)
                    channel = grpc.insecure_channel(address)
                    client = AxServeClient(channel)
                    client._managed_channel = channel
                    client._managed_process = process
                    self._clients[machine] = client
        client = self._clients[machine]
        return client


class AxServeClient:
    _instances_store: ClassVar[AxServeClientStore] = AxServeClientStore()

    _channel: Channel
    _timeout: float

    _stub: ActiveStub

    _instances_manager: AxServeInstancesManager
    _event_context_manager: AxServeEventContextManager
    _members_managers: AxServeMembersManagerCache
    _event_stream_manager: AxServeEventStreamManager | None = None
    _event_loop_manager: AxServeEventLoopManager | None = None

    _managed_channel: Channel | None = None
    _managed_process: AxServeServerProcess | None = None

    @classmethod
    def instance(cls, machine: str | None = None):
        return cls._instances_store.instance(machine)

    def __init__(
        self,
        channel: Channel,
        timeout: float | None = None,
    ) -> None:
        if not timeout:
            timeout = 15

        self._channel = channel
        self._timeout = timeout

        self._stub = ActiveStub(channel)
        self._instances_manager = AxServeInstancesManager()
        self._event_context_manager = AxServeEventContextManager()
        self._members_managers = AxServeMembersManagerCache(
            self._stub,
            self._event_context_manager,
        )

        self.__enter__()

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

    def _create_internals(
        self, c: str, internals: AxServeObjectInternals | None = None
    ) -> AxServeObjectInternals:
        i = self._create_instance(c)
        members_manager = self._members_managers._get_members_manager(c, i)
        event_handlers_manager = AxServeEventHandlersManager()
        if not internals:
            internals = AxServeObjectInternals()
        internals._instance = i
        internals._clsid = c
        internals._client = self
        internals._members_manager = members_manager
        internals._event_handlers_manager = event_handlers_manager
        return internals

    def _initialize_internals(self, o: AxServeObject, c: str) -> None:
        i = o.__axserve__
        i = self._create_internals(c, i)
        o.__dict__["__axserve__"] = i  # skip __setattr__
        instance = typing.cast(str, i._instance)
        self._instances_manager._register_instance(instance, o)

    def create(self, c: str) -> AxServeObject:
        instance = AxServeObject(c, client=self)
        return instance

    def destroy(self, o: AxServeObject) -> None:
        if (
            (ax := o.__axserve__)
            and (instance := ax._instance)
            and self._instances_manager._has_instance(instance)
        ):
            if not self._destroy_instance(instance):
                msg = "Failed to destroy the axserve object"
                raise RuntimeError(msg)

    def close(self, timeout: float | None = None) -> None:
        start_time = time.time()
        if self._event_loop_manager:
            self._event_loop_manager.stop()
            self._event_loop_manager = None
        if self._event_stream_manager:
            self._event_stream_manager._close_event_stream()
            self._event_stream_manager = None
        if self._managed_channel:
            self._managed_channel.close()
            self._managed_channel = None
        elapsed_time = time.time() - start_time
        timeout = timeout - elapsed_time if timeout is not None else None
        if self._managed_process:
            self._managed_process.terminate()
            self._managed_process.wait(timeout=timeout)
            self._managed_process = None

    def __finalize__(self):
        self.close()

    def __enter__(self):
        grpc.channel_ready_future(self._channel).result(timeout=self._timeout)

        if not self._event_stream_manager:
            self._event_stream_manager = AxServeEventStreamManager(self._stub)
        if not self._event_loop_manager:
            self._event_loop_manager = AxServeEventLoopManager(
                self._instances_manager,
                self._event_context_manager,
                self._event_stream_manager,
            )

        if not self._event_loop_manager.is_running():
            self._event_loop_manager.start()

        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        exc_traceback: TracebackType | None,
    ) -> None:
        self.__finalize__()


class AxServeObject:
    __axserve__: AxServeObjectInternals | None = None

    def __init__(
        self,
        c: str | None = None,
        *,
        client: AxServeClient | None = None,
    ) -> None:
        if not c and hasattr(self, "__CLSID__"):
            c = self.__CLSID__
        if not self.__axserve__:
            self.__axserve__ = AxServeObjectInternals(c, client=client)
        elif self.__axserve__._instance:
            return
        clsid = self.__axserve__._clsid
        client = self.__axserve__._client
        if not clsid:
            msg = "Cannot determine CLSID"
            raise ValueError(msg)
        if not client:
            machine = CheckMachineFromCLSID(clsid)
            client = AxServeClient.instance(machine)
        self.__axserve__._clsid = clsid
        self.__axserve__._client = client
        client._initialize_internals(self, clsid)

    def __getitem__(self, name) -> AxServeMemberType:
        if (
            (ax := self.__axserve__)
            and (mm := ax._members_manager)
            and mm._has_member_name(name)
        ):
            member = mm._get_member_by_name(name)
            return AxServeMemberType(member, self)
        raise KeyError(name)

    def __getattr__(self, name):
        if (
            (ax := self.__axserve__)
            and (mm := ax._members_manager)
            and mm._has_member_name(name)
        ):
            return mm._get_member_by_name(name).__get__(self)
        return super().__getattribute__(name)

    def __setattr__(self, name, value):  # type: ignore
        if (
            (ax := self.__axserve__)
            and (mm := ax._members_manager)
            and mm._has_member_name(name)
        ):
            return mm._get_member_by_name(name).__set__(self, value)
        return super().__setattr__(name, value)

    def __dir__(self) -> Iterable[str]:
        if (ax := self.__axserve__) and (mm := ax._members_manager):
            members = mm._get_member_names()
            attrs = super().__dir__()
            attrs = set(attrs) | set(members)
            attrs = list(attrs)
            return attrs
        return super().__dir__()

    def __finalize__(self):
        if (
            (ax := self.__axserve__)
            and (client := ax._client)
            and (instance := ax._instance)
        ):
            client._instances_manager._unregister_instance(instance)
            client._destroy_instance(instance)
            self.__axserve__._instance = None

    def __enter__(self):
        if not self.__axserve__:
            self.__axserve__ = AxServeObjectInternals()
        self.__init__(
            self.__axserve__._clsid,
            client=self.__axserve__._client,
        )
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc_value: BaseException | None,
        exc_traceback: TracebackType | None,
    ) -> None:
        self.__finalize__()
