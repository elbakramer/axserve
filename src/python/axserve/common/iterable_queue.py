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

from typing import Iterable
from typing import Iterator
from typing import Optional
from typing import TypeVar

from axserve.common.closeable_queue import CloseableQueue
from axserve.common.closeable_queue import Closed


T = TypeVar("T")


class IterableQueue(CloseableQueue[T], Iterable[T]):
    def next(self, timeout: Optional[float] = None) -> T:
        try:
            return self.get(timeout=timeout)
        except Closed as exc:
            raise StopIteration from exc

    def __iter__(self) -> Iterator[T]:
        return self

    def __next__(self) -> T:
        return self.next()
