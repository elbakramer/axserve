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
import platform

from asyncio import Lock
from typing import TYPE_CHECKING
from typing import ClassVar

import grpc

from axserve.aio.client.component import AxServeEventContextManager
from axserve.aio.client.component import AxServeEventHandlersManager
from axserve.aio.client.component import AxServeEventLoopManager
from axserve.aio.client.component import AxServeEventStreamManager
from axserve.aio.client.component import AxServeInstancesManager
from axserve.aio.client.component import AxServeMembersManager
from axserve.aio.client.component import AxServeMembersManagerCache
from axserve.aio.client.descriptor import AxServeMemberType
from axserve.aio.common.async_initializable import AsyncInitializable
from axserve.aio.server.process import AxServeServerProcess
from axserve.common.local import LoopLocal
from axserve.common.registry import check_machine_for_clsid
from axserve.common.socket import find_free_port
from axserve.proto import active_pb2
from axserve.proto.active_pb2_grpc import ActiveAsyncStub
from axserve.proto.active_pb2_grpc import ActiveStub


if TYPE_CHECKING:
    from collections.abc import Iterable
    from collections.abc import MutableMapping

    from grpc.aio import Channel


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


class AxServeClientStore(LoopLocal):
    _clients: MutableMapping[str, AxServeClient]
    _clients_lock: Lock

    def __init__(self):
        self._clients = {}
        self._clients_lock = Lock()

    async def instance(self, machine: str | None = None):
        if not machine:
            machine = platform.machine()
        if machine not in self._clients:
            async with self._clients_lock:
                if machine not in self._clients:
                    port = find_free_port()
                    address = f"localhost:{port}"
                    process = await AxServeServerProcess(address, machine=machine)
                    channel = grpc.aio.insecure_channel(address)
                    instance = await AxServeClient(channel)
                    instance._managed_channel = channel
                    instance._managed_process = process
                    self._clients[machine] = instance
        instance = self._clients[machine]
        return instance


class AxServeClient(AsyncInitializable["AxServeClient"]):
    _instances_store: ClassVar[AxServeClientStore] = AxServeClientStore()

    _channel: Channel
    _timeout: float

    _stub: ActiveAsyncStub

    _instances_manager: AxServeInstancesManager
    _event_context_manager: AxServeEventContextManager
    _members_managers: AxServeMembersManagerCache
    _event_stream_manager: AxServeEventStreamManager | None = None
    _event_loop_manager: AxServeEventLoopManager | None = None

    _managed_channel: Channel | None = None
    _managed_process: AxServeServerProcess | None = None

    @classmethod
    async def instance(cls, machine: str | None = None):
        return await cls._instances_store.instance(machine)

    def __init__(
        self,
        channel: Channel,
        timeout: float | None = None,
    ) -> None:
        if not timeout:
            timeout = 15

        self._channel = channel
        self._timeout = timeout

        self._stub = ActiveStub(self._channel)  # type:ignore

        self._instances_manager = AxServeInstancesManager()
        self._event_context_manager = AxServeEventContextManager()
        self._members_managers = AxServeMembersManagerCache(
            self._stub,
            self._event_context_manager,
        )

    async def __ainit__(self) -> None:
        async with asyncio.timeout(self._timeout):
            await self._channel.channel_ready()

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

    async def _create_instance(self, c: str) -> str:
        request = active_pb2.CreateRequest()
        request.clsid = c
        response = await self._stub.Create(request)
        instance = response.instance
        return instance

    async def _destroy_instance(self, i: str) -> bool:
        request = active_pb2.DestroyRequest()
        request.instance = i
        response = await self._stub.Destroy(request)
        return response.successful

    async def _create_internals(
        self, c: str, internals: AxServeObjectInternals | None = None
    ) -> AxServeObjectInternals:
        i = await self._create_instance(c)
        members_manager = await self._members_managers._get_members_manager(c, i)
        event_handlers_manager = AxServeEventHandlersManager()
        if not internals:
            internals = AxServeObjectInternals()
        internals._clsid = c
        internals._client = self
        internals._instance = i
        internals._members_manager = members_manager
        internals._event_handlers_manager = event_handlers_manager
        return internals

    async def _initialize_internals(self, o: AxServeObject, c: str) -> None:
        i = o.__axserve__
        i = await self._create_internals(c, i)
        o.__dict__["__axserve__"] = i  # skip __setattr__
        if not i._instance:
            msg = "Instance id is empty"
            raise ValueError(msg)
        self._instances_manager._register_instance(i._instance, o)

    async def create(self, c: str) -> AxServeObject:
        instance = AxServeObject(c, client=self)
        return await instance

    async def destroy(self, o: AxServeObject) -> None:
        await o.__afinalize__()

    async def close(self, timeout: float | None = None) -> None:
        async with asyncio.timeout(timeout):
            if self._event_loop_manager:
                await self._event_loop_manager.stop()
                self._event_loop_manager = None
            if self._event_stream_manager:
                await self._event_stream_manager._close_event_stream()
                self._event_stream_manager = None
            if self._managed_channel:
                await self._managed_channel.close()
                self._managed_channel = None
            if self._managed_process:
                try:
                    self._managed_process.terminate()
                except ProcessLookupError:
                    pass
                else:
                    await self._managed_process.wait()
                finally:
                    self._managed_process = None

    async def __afinalize__(self) -> None:
        await self.close()


class AxServeObject(AsyncInitializable["AxServeObject"]):
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

    async def __ainit__(self) -> None:
        if not self.__axserve__:
            self.__axserve__ = AxServeObjectInternals()
        elif self.__axserve__._instance:
            return
        clsid = self.__axserve__._clsid
        client = self.__axserve__._client
        if not clsid:
            msg = "Cannot determine CLSID"
            raise ValueError(msg)
        if not client:
            machine = check_machine_for_clsid(clsid)
            client = await AxServeClient.instance(machine)
        self.__axserve__._clsid = clsid
        self.__axserve__._client = client
        await client._initialize_internals(self, clsid)

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
            and (mm._has_member_name(name))
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

    async def __afinalize__(self):
        if (
            (ax := self.__axserve__)
            and (client := ax._client)
            and (instance := ax._instance)
        ):
            client._instances_manager._unregister_instance(instance)
            await client._destroy_instance(instance)
