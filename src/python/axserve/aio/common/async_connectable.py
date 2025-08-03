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

from abc import abstractmethod
from typing import TYPE_CHECKING
from typing import Any
from typing import ParamSpec
from typing import Protocol
from typing import TypeVar

from axserve.common.protocol import check_names_in_mro


if TYPE_CHECKING:
    from collections.abc import Callable


P = ParamSpec("P")

C_co = TypeVar("C_co", covariant=True)
D_co = TypeVar("D_co", covariant=True)


class AsyncConnectable(Protocol[P, C_co, D_co]):
    @abstractmethod
    async def connect(self, handler: Callable[P, Any]) -> C_co | None:
        raise NotImplementedError

    @abstractmethod
    async def disconnect(self, handler: Callable[P, Any]) -> D_co | None:
        raise NotImplementedError

    @classmethod
    def __subclasshook__(cls, __subclass: type, /) -> bool:
        if cls is AsyncConnectable and check_names_in_mro(
            ["connect", "disconnect"], __subclass
        ):
            return True
        return super().__subclasshook__(__subclass)
