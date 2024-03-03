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

from abc import ABC
from abc import abstractmethod
from collections.abc import Callable
from collections.abc import Iterator
from typing import TYPE_CHECKING

import grpc


if TYPE_CHECKING:
    from axserve.client.descriptor import AxServeEvent
    from axserve.client.descriptor import AxServeMember
    from axserve.client.descriptor import AxServeMemberType
    from axserve.client.descriptor import AxServeMethod
    from axserve.client.descriptor import AxServeProperty
    from axserve.common.aquireable import Aquireable
    from axserve.proto import active_pb2
    from axserve.proto import active_pb2_grpc


class AbstractAxServeStubProvider(ABC):
    @abstractmethod
    def _get_stub(self) -> active_pb2_grpc.ActiveStub:
        raise NotImplementedError()


class AbstractAxServeStubManager(AbstractAxServeStubProvider):
    @abstractmethod
    def _get_channel(self) -> grpc.Channel | None:
        raise NotImplementedError()

    @abstractmethod
    def _get_managed_channel(self) -> grpc.Channel | None:
        raise NotImplementedError()

    @abstractmethod
    def _close_managed_channel(self) -> None:
        raise NotImplementedError()

    @abstractmethod
    def _wait_until_channel_ready(self, timeout: float | None = None) -> None:
        raise NotImplementedError()


class AbstractAxServeCurrentHandleEventProvider(ABC):
    @abstractmethod
    def _get_current_handle_event(self) -> active_pb2.HandleEventRequest | None:
        raise NotImplementedError()


class AbstractAxServeEventContextManager(AbstractAxServeCurrentHandleEventProvider):
    @abstractmethod
    def _get_handle_event_context_stack(self) -> list[active_pb2.HandleEventRequest]:
        raise NotImplementedError()


class AbstractAxServeEventHandlersManager(ABC):
    @abstractmethod
    def _get_event_handlers(self, index: int) -> list[Callable]:
        raise NotImplementedError()

    @abstractmethod
    def _get_event_handlers_lock(self, index: int) -> Aquireable:
        raise NotImplementedError()


class AbstractAxServeEventStreamManager(ABC):
    @abstractmethod
    def _get_handle_event_requests(self) -> Iterator[active_pb2.HandleEventRequest]:
        raise NotImplementedError()

    @abstractmethod
    def _put_handle_event_response(self, response: active_pb2.HandleEventResponse) -> None:
        raise NotImplementedError()

    @abstractmethod
    def _close_event_stream(self) -> None:
        raise NotImplementedError()

    @abstractmethod
    def _cancel_event_stream(self) -> None:
        raise NotImplementedError()


class AbstractAxServeMembersManager(ABC):
    @abstractmethod
    def _get_property(self, index: int) -> AxServeProperty:
        raise NotImplementedError()

    @abstractmethod
    def _get_method(self, index: int) -> AxServeMethod:
        raise NotImplementedError()

    @abstractmethod
    def _get_event(self, index: int) -> AxServeEvent:
        raise NotImplementedError()

    @abstractmethod
    def _has_member_name(self, name: str) -> bool:
        raise NotImplementedError()

    @abstractmethod
    def _get_member_by_name(self, name: str) -> AxServeMember:
        raise NotImplementedError()

    @abstractmethod
    def _get_member_names(self) -> list[str]:
        raise NotImplementedError()


class AbstractAxServeEventLoop(ABC):
    @abstractmethod
    def exec(self) -> int:  # noqa: A003
        raise NotImplementedError()

    @abstractmethod
    def is_running(self) -> bool:
        raise NotImplementedError()

    @abstractmethod
    def wake_up(self) -> None:
        raise NotImplementedError()

    @abstractmethod
    def exit(self, return_code: int = 0) -> None:  # noqa: A003
        raise NotImplementedError()


class AbstractAxServeEventLoopManager(ABC):
    @abstractmethod
    def start(self):
        raise NotImplementedError()

    @abstractmethod
    def stop(self):
        raise NotImplementedError()


class AbstractAxServeObject(
    AbstractAxServeStubProvider,
    AbstractAxServeMembersManager,
    AbstractAxServeEventContextManager,
    AbstractAxServeEventHandlersManager,
    AbstractAxServeEventStreamManager,
):
    @abstractmethod
    def __getitem__(self, name) -> AxServeMemberType:
        raise NotImplementedError()

    @abstractmethod
    def close(self):
        raise NotImplementedError()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
