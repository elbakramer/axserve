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
from typing import Protocol

from axserve.common.protocol import check_names_in_mro


class Aquireable(Protocol):
    @abstractmethod
    def aquire(self) -> bool:
        raise NotImplementedError()

    @abstractmethod
    def release(self) -> None:
        raise NotImplementedError()

    @abstractmethod
    def __enter__(self) -> bool:
        raise NotImplementedError()

    @abstractmethod
    def __exit__(self, exc_type, exc_value, traceback) -> None:
        raise NotImplementedError()

    @classmethod
    def __subclasshook__(cls, __subclass: type) -> bool:
        if cls is Aquireable:
            if check_names_in_mro(["aquire", "release", "__enter__", "__exit__"], __subclass):
                return True
        return super().__subclasshook__(__subclass)
