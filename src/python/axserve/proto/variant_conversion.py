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

import inspect

from typing import Any

from axserve.proto import active_pb2


def NoneFromVariant(variant: active_pb2.Variant) -> None:
    return None


def BoolFromVariant(variant: active_pb2.Variant) -> bool:
    return variant.bool_value


def StringFromVariant(variant: active_pb2.Variant) -> str:
    return variant.string_value


def Int32FromVariant(variant: active_pb2.Variant) -> int:
    return variant.int_value


def UInt32FromVariant(variant: active_pb2.Variant) -> int:
    return variant.uint_value


def DoubleFromVariant(variant: active_pb2.Variant) -> float:
    return variant.double_value


def ListFromVariant(variant: active_pb2.Variant) -> list:
    return [ValueFromVariant(value) for value in variant.list_value.values]


def MapFromVariant(variant: active_pb2.Variant) -> dict:
    return {name: ValueFromVariant(value) for name, value in variant.map_value.values.items()}


ValueFromVariant_Methods = {
    None: NoneFromVariant,
    "bool_value": BoolFromVariant,
    "string_value": StringFromVariant,
    "int_value": Int32FromVariant,
    "uint_value": UInt32FromVariant,
    "double_value": DoubleFromVariant,
    "list_value": ListFromVariant,
    "map_value": MapFromVariant,
}


def ValueFromVariant(variant: active_pb2.Variant) -> Any:
    return ValueFromVariant_Methods[variant.WhichOneof("value")](variant)


def ValueToVariant(
    value: Any,
    variant: active_pb2.Variant | None = None,
) -> active_pb2.Variant:
    if variant is None:
        variant = active_pb2.Variant()
    if value is None:
        pass
    elif isinstance(value, bool):
        variant.bool_value = value
    elif isinstance(value, str):
        variant.string_value = value
    elif isinstance(value, int):
        variant.int_value = value
    elif isinstance(value, float):
        variant.double_value = value
    elif isinstance(value, list):
        for value_item in value:
            variant_item = variant.list_value.values.add()
            ValueToVariant(value_item, variant_item)
    elif isinstance(value, dict):
        for value_name, value_value in value.items():
            ValueToVariant(value_value, variant.map_value.values[value_name])
    else:
        msg = f"Unexpected value type: {type(value)}"
        raise TypeError(msg)
    return variant


AnnotationFromTypeName_Annotations = {
    "void": None,
    "bool": bool,
    "QString": str,
    "int": int,
    "unsigned int": int,
    "double": float,
    "QVariant": inspect.Parameter.empty,
    "QVariantList": list,
    "QVariantMap": map,
    "QVariantHash": map,
}


def AnnotationFromTypeName(type_name: str) -> Any:
    return AnnotationFromTypeName_Annotations.get(type_name, inspect.Parameter.empty)
