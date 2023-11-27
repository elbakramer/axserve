from __future__ import annotations

import atexit
import inspect
import threading

from concurrent.futures import Future
from concurrent.futures import ThreadPoolExecutor
from queue import Empty
from queue import Full
from queue import Queue
from threading import Thread
from time import time
from typing import Any
from typing import Callable
from typing import Iterable
from typing import Iterator
from typing import Optional
from typing import TypeVar
from typing import Union
from typing import cast

import grpc

from . import active_pb2
from . import active_pb2_grpc


T = TypeVar("T")


class Closeable:
    def close(self) -> None:
        raise NotImplementedError()


class Closed(Exception):
    pass


class CloseableQueue(Queue[T], Closeable):
    def __init__(self, *args: Any, **kwargs: Any) -> None:
        super().__init__(*args, **kwargs)
        assert not hasattr(self, "_closed")
        self._closed = False

    def put(
        self,
        item: T,
        block: bool = True,
        timeout: Optional[float] = None,
    ) -> None:
        with self.not_full:
            if self._closed:
                raise Closed
            if self.maxsize > 0:
                if not block:
                    if self._qsize() >= self.maxsize:
                        raise Full
                elif timeout is None:
                    while self._qsize() >= self.maxsize:
                        self.not_full.wait()
                elif timeout < 0:
                    raise ValueError("'timeout' must be a non-negative number")
                else:
                    endtime = time() + timeout
                    while self._qsize() >= self.maxsize:
                        remaining = endtime - time()
                        if remaining <= 0.0:
                            raise Full
                        self.not_full.wait(remaining)
            self._put(item)
            self.unfinished_tasks += 1  # pylint: disable=no-member
            self.not_empty.notify()

    def get(
        self,
        block: bool = True,
        timeout: Optional[float] = None,
    ) -> T:
        with self.not_empty:
            if self._closed and not self._qsize():
                raise Closed
            if not block:
                if not self._qsize():
                    raise Empty
            elif timeout is None:
                while not self._qsize():
                    self.not_empty.wait()
            elif timeout < 0:
                raise ValueError("'timeout' must be a non-negative number")
            else:
                endtime = time() + timeout
                while not self._qsize():
                    remaining = endtime - time()
                    if remaining <= 0.0:
                        raise Empty
                    self.not_empty.wait(remaining)
            item = self._get()
            self.not_full.notify()
            return item

    def close(
        self,
        block: bool = True,
        timeout: Optional[float] = None,
        idempotent: bool = True,
        immediate: bool = False,
    ) -> None:
        with self.not_full:
            if self._closed:
                if idempotent:
                    return
                raise Closed
            if self.maxsize > 0:
                if not block:
                    if self._qsize() >= self.maxsize:
                        if not immediate:
                            raise Full
                elif timeout is None:
                    while self._qsize() >= self.maxsize:
                        if immediate:
                            break
                        self.not_full.wait()
                elif timeout < 0:
                    raise ValueError("'timeout' must be a non-negative number")
                else:
                    endtime = time() + timeout
                    while self._qsize() >= self.maxsize:
                        remaining = endtime - time()
                        if remaining <= 0.0:
                            if immediate:
                                break
                            raise Full
                        self.not_full.wait(remaining)
            self._closed = True
            self.not_empty.notify_all()
            if immediate:
                self.not_full.notify_all()

    def closed(self) -> bool:
        with self.mutex:
            return self._closed


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


def ValueToVariant(value: Any, variant: active_pb2.Variant) -> active_pb2.Variant:
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
        raise TypeError(f"Unexpected type {type(value)}")
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


class AxServeProperty:
    def __init__(
        self,
        obj: AxServeObject,
        info: active_pb2.PropertyInfo,
    ):
        self._obj = obj
        self._info = info

    def __set_name__(self, owner, name):
        pass

    def __get__(self, obj: AxServeObject, objtype=None):
        if obj is None:
            return self
        request = active_pb2.GetPropertyRequest()
        request.index = self._info.index
        obj._set_request_context(request)
        response = obj._stub.GetProperty(request)
        response = cast(active_pb2.GetPropertyResponse, response)
        return ValueFromVariant(response.value)

    def __set__(self, obj: AxServeObject, value: Any):
        request = active_pb2.SetPropertyRequest()
        request.index = self._info.index
        ValueToVariant(value, request.value)
        obj._set_request_context(request)
        response = obj._stub.SetProperty(request)
        response = cast(active_pb2.SetPropertyResponse, response)
        assert response is not None


class AxServeMethod:
    def __init__(
        self,
        obj: AxServeObject,
        info: active_pb2.MethodInfo,
    ):
        self._obj = obj
        self._info = info
        self._sig = inspect.Signature(
            parameters=[
                inspect.Parameter(
                    name=arg.name,
                    kind=inspect.Parameter.POSITIONAL_OR_KEYWORD,
                    annotation=AnnotationFromTypeName(arg.argument_type),
                )
                for arg in self._info.arguments
            ],
            return_annotation=AnnotationFromTypeName(self._info.return_type),
        )
        self.__name__ = self._info.name
        self.__signature__ = self._sig

    def __call__(self, *args, **kwargs):
        request = active_pb2.InvokeMethodRequest()
        request.index = self._info.index
        bound_args = self._sig.bind(*args, **kwargs)
        bound_args.apply_defaults()
        for arg in bound_args.args:
            ValueToVariant(arg, request.arguments.add())
        self._obj._set_request_context(request)
        response = self._obj._stub.InvokeMethod(request)
        response = cast(active_pb2.InvokeMethodResponse, response)
        return ValueFromVariant(response.return_value)


class AxServeEvent:
    def __init__(
        self,
        obj: AxServeObject,
        info: active_pb2.EventInfo,
    ):
        self._obj = obj
        self._info = info
        self._sig = inspect.Signature(
            parameters=[
                inspect.Parameter(
                    name=arg.name,
                    kind=inspect.Parameter.POSITIONAL_OR_KEYWORD,
                    annotation=AnnotationFromTypeName(arg.argument_type),
                )
                for arg in self._info.arguments
            ],
            return_annotation=None,
        )
        self._handlers = []
        self._handlers_lock = threading.RLock()
        self.__name__ = self._info.name
        self.__signature__ = self._sig

    def connect(self, handler):
        with self._handlers_lock:
            if not self._handlers:
                request = active_pb2.ConnectEventRequest()
                request.index = self._info.index
                self._obj._set_request_context(request)
                response = self._obj._stub.ConnectEvent(request)
                response = cast(active_pb2.ConnectEventResponse, response)
                assert response.successful
            self._handlers.append(handler)

    def disconnect(self, handler):
        with self._handlers_lock:
            self._handlers.remove(handler)
            if not self._handlers:
                request = active_pb2.DisconnectEventRequest()
                request.index = self._info.index
                self._obj._set_request_context(request)
                response = self._obj._stub.DisconnectEvent(request)
                response = cast(active_pb2.DisconnectEventResponse, response)
                assert response.successful

    def __call__(self, *args, **kwargs):
        with self._handlers_lock:
            handlers = list(self._handlers)
        for handler in handlers:
            handler(*args, **kwargs)


class AxServeEventContext:
    _thread_local = threading.local()
    _thread_local._stack = []

    def __init__(self, obj: AxServeObject, handle: active_pb2.HandleEventRequest):
        self._obj = obj
        self._handle = handle

    @classmethod
    def _stack(cls) -> list[AxServeEventContext]:
        if not hasattr(cls._thread_local, "_stack"):
            cls._thread_local._stack = []
        return cls._thread_local._stack

    def __enter__(self):
        self._stack().append(self)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self._stack().pop()
        response = active_pb2.HandleEventResponse()
        response.index = self._handle.index
        response.id = self._handle.id
        self._obj._handle_event_response_queue.put(response)
        return


RequestsWithRequestContext = Union[
    active_pb2.GetPropertyRequest,
    active_pb2.SetPropertyRequest,
    active_pb2.InvokeMethodRequest,
    active_pb2.ConnectEventRequest,
    active_pb2.DisconnectEventRequest,
]


class AxServeObject:
    def __init__(
        self,
        channel: grpc.Channel,
        *,
        thread_constructor: Optional[Callable[..., Thread]] = None,
        thread_pool_executor: Optional[ThreadPoolExecutor] = None,
        expose_process_events_method: bool = False,
    ):
        self.__dict__["_properties_dict"] = {}
        self.__dict__["_methods_dict"] = {}
        self.__dict__["_events_dict"] = {}
        self._channel = channel
        self._stub = active_pb2_grpc.ActiveStub(channel)
        request = active_pb2.DescribeRequest()
        self._set_request_context(request)
        response = self._stub.Describe(request)
        response = cast(active_pb2.DescribeResponse, response)
        self._properties_list = []
        self._properties_dict = {}
        self._methods_list = []
        self._methods_dict = {}
        self._events_list = []
        self._events_dict = {}
        for info in response.properties:
            prop = AxServeProperty(self, info)
            self._properties_list.append(prop)
            self._properties_dict[info.name] = prop
        for info in response.methods:
            method = AxServeMethod(self, info)
            self._methods_list.append(method)
            self._methods_dict[info.name] = method
            setattr(self, info.name, method)
        for info in response.events:
            event = AxServeEvent(self, info)
            self._events_list.append(event)
            self._events_dict[info.name] = event
            setattr(self, info.name, event)
        self._handle_event_response_queue: Optional[IterableQueue] = None
        self._thread: Optional[Thread] = None
        self._future: Optional[Future] = None
        self._process_events_prop: Optional[Callable[[], None]] = None
        self._process_events_exception: Optional[Exception] = None
        atexit.register(self.close)
        self._handle_event_response_queue = IterableQueue()
        self._handle_events = self._stub.HandleEvent(self._handle_event_response_queue)
        self._handle_events = cast(
            Iterator[active_pb2.HandleEventRequest], self._handle_events
        )
        if expose_process_events_method:
            self._process_events_prop = self._process_handle_event_requests
        elif thread_pool_executor:
            self._future = thread_pool_executor.submit(
                self._process_handle_event_requests_target
            )
        else:
            if not thread_constructor:
                thread_constructor = threading.Thread
            self._thread = thread_constructor(
                target=self._process_handle_event_requests_target
            )
            self._thread.start()

    def __getattr__(self, name):
        if name in self._properties_dict:
            return self._properties_dict[name].__get__(self, type(self))
        return super().__getattribute__(name)

    def __setattr__(self, name, value):
        if name in self._properties_dict:
            return self._properties_dict[name].__set__(self, value)
        return super().__setattr__(name, value)

    def __dir__(self):
        props = list(self._properties_dict.keys())
        attrs = super().__dir__()
        return props + attrs

    def _enter_event_context(self, handle: active_pb2.HandleEventRequest):
        return AxServeEventContext(self, handle)

    def _set_request_context(self, request: RequestsWithRequestContext):
        event_context_stack = AxServeEventContext._stack()
        if event_context_stack:
            callback_event_index = event_context_stack[-1]._handle.index
            request.request_context = active_pb2.RequestContext.EVENT_CALLBACK
            request.callback_event_index = callback_event_index
        return request

    def _process_handle_event_requests(self):
        for handle_event in self._handle_events:
            with self._enter_event_context(handle_event):
                args = [ValueFromVariant(arg) for arg in handle_event.arguments]
                self._events_list[handle_event.index](*args)

    def _process_handle_event_requests_target(self):
        try:
            self._process_handle_event_requests()
        except grpc.RpcError as exc:
            self._process_events_exception = exc
            self = None
            if (
                isinstance(exc, grpc.Call)
                and exc.code() == grpc.StatusCode.CANCELLED
                and isinstance(exc, grpc.RpcContext)
                and not exc.is_active()
            ):
                return
            raise exc
        except Exception as exc:
            self._process_events_exception = exc
            self = None
            raise exc

    def process_events(self):
        if self._process_events_prop:
            return self._process_events_prop()

    def join(self, timeout: Optional[float] = None):
        if self._thread:
            self._thread.join(timeout=timeout)
        if self._future:
            self._future.result(timeout=timeout)

    def close(self, timeout: Optional[float] = None):
        if self._handle_event_response_queue:
            self._handle_event_response_queue.close()
        self.join(timeout=timeout)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
        return
