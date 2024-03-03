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


def check_name_in_mro(name: str, subclass: type) -> bool:
    return any(name in baseclass.__dict__ for baseclass in subclass.__mro__)


def check_names_in_mro(names: list[str], subclass: type) -> bool:
    return all(check_name_in_mro(name, subclass) for name in names)
