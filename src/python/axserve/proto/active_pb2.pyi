from google.protobuf.internal import containers as _containers
from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class RequestContext(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []
    DEFAULT: _ClassVar[RequestContext]
    EVENT_CALLBACK: _ClassVar[RequestContext]
DEFAULT: RequestContext
EVENT_CALLBACK: RequestContext

class DescribeRequest(_message.Message):
    __slots__ = ["request_context", "callback_event_index"]
    REQUEST_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    CALLBACK_EVENT_INDEX_FIELD_NUMBER: _ClassVar[int]
    request_context: RequestContext
    callback_event_index: int
    def __init__(self, request_context: _Optional[_Union[RequestContext, str]] = ..., callback_event_index: _Optional[int] = ...) -> None: ...

class PropertyInfo(_message.Message):
    __slots__ = ["index", "name", "property_type", "is_readable", "is_writable"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    NAME_FIELD_NUMBER: _ClassVar[int]
    PROPERTY_TYPE_FIELD_NUMBER: _ClassVar[int]
    IS_READABLE_FIELD_NUMBER: _ClassVar[int]
    IS_WRITABLE_FIELD_NUMBER: _ClassVar[int]
    index: int
    name: str
    property_type: str
    is_readable: bool
    is_writable: bool
    def __init__(self, index: _Optional[int] = ..., name: _Optional[str] = ..., property_type: _Optional[str] = ..., is_readable: bool = ..., is_writable: bool = ...) -> None: ...

class ArgumentInfo(_message.Message):
    __slots__ = ["name", "argument_type"]
    NAME_FIELD_NUMBER: _ClassVar[int]
    ARGUMENT_TYPE_FIELD_NUMBER: _ClassVar[int]
    name: str
    argument_type: str
    def __init__(self, name: _Optional[str] = ..., argument_type: _Optional[str] = ...) -> None: ...

class MethodInfo(_message.Message):
    __slots__ = ["index", "name", "arguments", "return_type"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    NAME_FIELD_NUMBER: _ClassVar[int]
    ARGUMENTS_FIELD_NUMBER: _ClassVar[int]
    RETURN_TYPE_FIELD_NUMBER: _ClassVar[int]
    index: int
    name: str
    arguments: _containers.RepeatedCompositeFieldContainer[ArgumentInfo]
    return_type: str
    def __init__(self, index: _Optional[int] = ..., name: _Optional[str] = ..., arguments: _Optional[_Iterable[_Union[ArgumentInfo, _Mapping]]] = ..., return_type: _Optional[str] = ...) -> None: ...

class EventInfo(_message.Message):
    __slots__ = ["index", "name", "arguments"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    NAME_FIELD_NUMBER: _ClassVar[int]
    ARGUMENTS_FIELD_NUMBER: _ClassVar[int]
    index: int
    name: str
    arguments: _containers.RepeatedCompositeFieldContainer[ArgumentInfo]
    def __init__(self, index: _Optional[int] = ..., name: _Optional[str] = ..., arguments: _Optional[_Iterable[_Union[ArgumentInfo, _Mapping]]] = ...) -> None: ...

class DescribeResponse(_message.Message):
    __slots__ = ["properties", "methods", "events"]
    PROPERTIES_FIELD_NUMBER: _ClassVar[int]
    METHODS_FIELD_NUMBER: _ClassVar[int]
    EVENTS_FIELD_NUMBER: _ClassVar[int]
    properties: _containers.RepeatedCompositeFieldContainer[PropertyInfo]
    methods: _containers.RepeatedCompositeFieldContainer[MethodInfo]
    events: _containers.RepeatedCompositeFieldContainer[EventInfo]
    def __init__(self, properties: _Optional[_Iterable[_Union[PropertyInfo, _Mapping]]] = ..., methods: _Optional[_Iterable[_Union[MethodInfo, _Mapping]]] = ..., events: _Optional[_Iterable[_Union[EventInfo, _Mapping]]] = ...) -> None: ...

class VariantList(_message.Message):
    __slots__ = ["values"]
    VALUES_FIELD_NUMBER: _ClassVar[int]
    values: _containers.RepeatedCompositeFieldContainer[Variant]
    def __init__(self, values: _Optional[_Iterable[_Union[Variant, _Mapping]]] = ...) -> None: ...

class VaraintHashMap(_message.Message):
    __slots__ = ["values"]
    class ValuesEntry(_message.Message):
        __slots__ = ["key", "value"]
        KEY_FIELD_NUMBER: _ClassVar[int]
        VALUE_FIELD_NUMBER: _ClassVar[int]
        key: str
        value: Variant
        def __init__(self, key: _Optional[str] = ..., value: _Optional[_Union[Variant, _Mapping]] = ...) -> None: ...
    VALUES_FIELD_NUMBER: _ClassVar[int]
    values: _containers.MessageMap[str, Variant]
    def __init__(self, values: _Optional[_Mapping[str, Variant]] = ...) -> None: ...

class Variant(_message.Message):
    __slots__ = ["bool_value", "string_value", "int_value", "uint_value", "double_value", "list_value", "map_value"]
    BOOL_VALUE_FIELD_NUMBER: _ClassVar[int]
    STRING_VALUE_FIELD_NUMBER: _ClassVar[int]
    INT_VALUE_FIELD_NUMBER: _ClassVar[int]
    UINT_VALUE_FIELD_NUMBER: _ClassVar[int]
    DOUBLE_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_VALUE_FIELD_NUMBER: _ClassVar[int]
    MAP_VALUE_FIELD_NUMBER: _ClassVar[int]
    bool_value: bool
    string_value: str
    int_value: int
    uint_value: int
    double_value: float
    list_value: VariantList
    map_value: VaraintHashMap
    def __init__(self, bool_value: bool = ..., string_value: _Optional[str] = ..., int_value: _Optional[int] = ..., uint_value: _Optional[int] = ..., double_value: _Optional[float] = ..., list_value: _Optional[_Union[VariantList, _Mapping]] = ..., map_value: _Optional[_Union[VaraintHashMap, _Mapping]] = ...) -> None: ...

class GetPropertyRequest(_message.Message):
    __slots__ = ["index", "request_context", "callback_event_index"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    REQUEST_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    CALLBACK_EVENT_INDEX_FIELD_NUMBER: _ClassVar[int]
    index: int
    request_context: RequestContext
    callback_event_index: int
    def __init__(self, index: _Optional[int] = ..., request_context: _Optional[_Union[RequestContext, str]] = ..., callback_event_index: _Optional[int] = ...) -> None: ...

class GetPropertyResponse(_message.Message):
    __slots__ = ["value"]
    VALUE_FIELD_NUMBER: _ClassVar[int]
    value: Variant
    def __init__(self, value: _Optional[_Union[Variant, _Mapping]] = ...) -> None: ...

class SetPropertyRequest(_message.Message):
    __slots__ = ["index", "value", "request_context", "callback_event_index"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    VALUE_FIELD_NUMBER: _ClassVar[int]
    REQUEST_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    CALLBACK_EVENT_INDEX_FIELD_NUMBER: _ClassVar[int]
    index: int
    value: Variant
    request_context: RequestContext
    callback_event_index: int
    def __init__(self, index: _Optional[int] = ..., value: _Optional[_Union[Variant, _Mapping]] = ..., request_context: _Optional[_Union[RequestContext, str]] = ..., callback_event_index: _Optional[int] = ...) -> None: ...

class SetPropertyResponse(_message.Message):
    __slots__ = []
    def __init__(self) -> None: ...

class InvokeMethodRequest(_message.Message):
    __slots__ = ["index", "arguments", "request_context", "callback_event_index"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    ARGUMENTS_FIELD_NUMBER: _ClassVar[int]
    REQUEST_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    CALLBACK_EVENT_INDEX_FIELD_NUMBER: _ClassVar[int]
    index: int
    arguments: _containers.RepeatedCompositeFieldContainer[Variant]
    request_context: RequestContext
    callback_event_index: int
    def __init__(self, index: _Optional[int] = ..., arguments: _Optional[_Iterable[_Union[Variant, _Mapping]]] = ..., request_context: _Optional[_Union[RequestContext, str]] = ..., callback_event_index: _Optional[int] = ...) -> None: ...

class InvokeMethodResponse(_message.Message):
    __slots__ = ["return_value"]
    RETURN_VALUE_FIELD_NUMBER: _ClassVar[int]
    return_value: Variant
    def __init__(self, return_value: _Optional[_Union[Variant, _Mapping]] = ...) -> None: ...

class ConnectEventRequest(_message.Message):
    __slots__ = ["index", "request_context", "callback_event_index"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    REQUEST_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    CALLBACK_EVENT_INDEX_FIELD_NUMBER: _ClassVar[int]
    index: int
    request_context: RequestContext
    callback_event_index: int
    def __init__(self, index: _Optional[int] = ..., request_context: _Optional[_Union[RequestContext, str]] = ..., callback_event_index: _Optional[int] = ...) -> None: ...

class ConnectEventResponse(_message.Message):
    __slots__ = ["successful"]
    SUCCESSFUL_FIELD_NUMBER: _ClassVar[int]
    successful: bool
    def __init__(self, successful: bool = ...) -> None: ...

class DisconnectEventRequest(_message.Message):
    __slots__ = ["index", "request_context", "callback_event_index"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    REQUEST_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    CALLBACK_EVENT_INDEX_FIELD_NUMBER: _ClassVar[int]
    index: int
    request_context: RequestContext
    callback_event_index: int
    def __init__(self, index: _Optional[int] = ..., request_context: _Optional[_Union[RequestContext, str]] = ..., callback_event_index: _Optional[int] = ...) -> None: ...

class DisconnectEventResponse(_message.Message):
    __slots__ = ["successful"]
    SUCCESSFUL_FIELD_NUMBER: _ClassVar[int]
    successful: bool
    def __init__(self, successful: bool = ...) -> None: ...

class HandleEventRequest(_message.Message):
    __slots__ = ["index", "id", "arguments", "is_ping", "is_pong"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    ID_FIELD_NUMBER: _ClassVar[int]
    ARGUMENTS_FIELD_NUMBER: _ClassVar[int]
    IS_PING_FIELD_NUMBER: _ClassVar[int]
    IS_PONG_FIELD_NUMBER: _ClassVar[int]
    index: int
    id: int
    arguments: _containers.RepeatedCompositeFieldContainer[Variant]
    is_ping: bool
    is_pong: bool
    def __init__(self, index: _Optional[int] = ..., id: _Optional[int] = ..., arguments: _Optional[_Iterable[_Union[Variant, _Mapping]]] = ..., is_ping: bool = ..., is_pong: bool = ...) -> None: ...

class HandleEventResponse(_message.Message):
    __slots__ = ["index", "id", "is_ping", "is_pong"]
    INDEX_FIELD_NUMBER: _ClassVar[int]
    ID_FIELD_NUMBER: _ClassVar[int]
    IS_PING_FIELD_NUMBER: _ClassVar[int]
    IS_PONG_FIELD_NUMBER: _ClassVar[int]
    index: int
    id: int
    is_ping: bool
    is_pong: bool
    def __init__(self, index: _Optional[int] = ..., id: _Optional[int] = ..., is_ping: bool = ..., is_pong: bool = ...) -> None: ...
