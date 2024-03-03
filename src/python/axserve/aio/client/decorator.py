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
from typing import Concatenate
from typing import ParamSpec
from typing import TypeVar

from axserve.aio.client.descriptor import AxServeEvent
from axserve.aio.client.descriptor import AxServeMethod
from axserve.aio.client.descriptor import AxServeProperty


T = TypeVar("T")
P = ParamSpec("P")
R = TypeVar("R")


def event(f: Callable[Concatenate[T, P], R]):
    return AxServeEvent(f)


def method(f: Callable[Concatenate[T, P], R]):
    return AxServeMethod(f)


def property(f: Callable[Concatenate[T, P], R]):
    return AxServeProperty(f)
