from __future__ import annotations

import inspect

from typing import Any
from typing import Optional

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
    return {
        name: ValueFromVariant(value)
        for name, value in variant.map_value.values.items()
    }


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
    variant: Optional[active_pb2.Variant] = None,
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
        raise TypeError(f"Unexpected value type: {type(value)}")
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
    return AnnotationFromTypeName_Annotations[type_name]
