from __future__ import annotations

import atexit
import contextlib
import inspect
import threading
import typing

from concurrent.futures import Future
from concurrent.futures import ThreadPoolExecutor
from threading import Thread
from typing import Any
from typing import Callable
from typing import Iterator
from typing import Optional
from typing import Union

import grpc
import pywintypes

from axserve.common.iterable_queue import IterableQueue
from axserve.common.socket import FindFreePort
from axserve.proto import active_pb2
from axserve.proto import active_pb2_grpc
from axserve.proto.variant_conversion import AnnotationFromTypeName
from axserve.proto.variant_conversion import ValueFromVariant
from axserve.proto.variant_conversion import ValueToVariant
from axserve.server.process import AxServeServerProcess


class AxServeProperty:
    def __init__(
        self,
        obj: AxServeObject,
        info: active_pb2.PropertyInfo,
    ):
        self._obj = obj
        self._info = info
        self._method: Optional[AxServeMethod] = None

    def __set_name__(self, owner, name):
        pass

    def __get__(self, obj: AxServeObject, objtype=None):
        if obj is None:
            return self
        request = active_pb2.GetPropertyRequest()
        request.index = self._info.index
        obj._set_request_context(request)
        response = obj._stub.GetProperty(request)
        response = typing.cast(active_pb2.GetPropertyResponse, response)
        return ValueFromVariant(response.value)

    def __set__(self, obj: AxServeObject, value: Any):
        request = active_pb2.SetPropertyRequest()
        request.index = self._info.index
        ValueToVariant(value, request.value)
        obj._set_request_context(request)
        response = obj._stub.SetProperty(request)
        response = typing.cast(active_pb2.SetPropertyResponse, response)
        assert response is not None

    def __call__(self, *args, **kwargs):
        if self._method:
            return self._method(*args, **kwargs)
        raise NotImplementedError()


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
        response = typing.cast(active_pb2.InvokeMethodResponse, response)
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
                response = typing.cast(active_pb2.ConnectEventResponse, response)
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
                response = typing.cast(active_pb2.DisconnectEventResponse, response)
                assert response.successful

    def __call__(self, *args, **kwargs):
        with self._handlers_lock:
            handlers = list(self._handlers)
        for handler in handlers:
            handler(*args, **kwargs)


class AxServeEventLoop:
    def __init__(self, obj: AxServeObject):
        self._obj = obj
        self._return_code = 0
        self._lock = threading.RLock()
        self._is_exitting = False
        self._is_running = False

    @contextlib.contextmanager
    def _create_exec_context(self):
        with self._lock:
            self._is_exitting = False
            self._is_running = True
        try:
            yield
        finally:
            with self._lock:
                self._is_exitting = False
                self._is_running = False

    @contextlib.contextmanager
    def _create_handle_event_context(self, handle_event: active_pb2.HandleEventRequest):
        event_context_stack = self._obj._get_handle_event_context_stack()
        event_context_stack.append(handle_event)
        try:
            yield
        finally:
            event_context_stack = self._obj._get_handle_event_context_stack()
            event_context_stack.pop()
            response = active_pb2.HandleEventResponse()
            response.index = handle_event.index
            response.id = handle_event.id
            self._obj._handle_event_response_queue.put(response)

    def exec(self) -> int:
        with self._create_exec_context():
            if not self._obj._handle_event_requests:
                if not self._obj._handle_event_response_queue or self._obj._handle_event_response_queue.closed():
                    self._obj._handle_event_response_queue = IterableQueue()
                self._obj._handle_event_requests = self._obj._stub.HandleEvent(self._obj._handle_event_response_queue)

            handle_events = typing.cast(
                Iterator[active_pb2.HandleEventRequest],
                self._obj._handle_event_requests,
            )

            try:
                for handle_event in handle_events:
                    with self._create_handle_event_context(handle_event):
                        args = [ValueFromVariant(arg) for arg in handle_event.arguments]
                        self._obj._events_list[handle_event.index](*args)
            except grpc.RpcError as exc:
                if not (self._is_exitting and isinstance(exc, grpc.Call) and exc.code() == grpc.StatusCode.CANCELLED):
                    raise exc

        return self._return_code

    def is_running(self) -> bool:
        return self._is_running

    def wake_up(self) -> None:
        state = getattr(self._obj._handle_event_requests, "_state", None)
        condition = getattr(state, "condition", None)
        condition = typing.cast(Optional[threading.Condition], condition)
        if condition is not None:
            with condition:
                condition.notify_all()

    def exit(self, return_code: int = 0) -> None:
        with self._lock:
            if not self._is_running:
                return
            if self._is_exitting:
                return
            self._is_exitting = True
            self._return_code = return_code
        if self._obj._handle_event_response_queue:
            self._obj._handle_event_response_queue.close()
        if self._obj._handle_event_requests:
            handle_events = typing.cast(grpc.RpcContext, self._obj._handle_event_requests)
            handle_events.cancel()

    def quit(self) -> None:
        self.exit()


class AxServeLibraryInfo:
    pass


AxServeCommonRequest = Union[
    active_pb2.DescribeRequest,
    active_pb2.GetPropertyRequest,
    active_pb2.SetPropertyRequest,
    active_pb2.InvokeMethodRequest,
    active_pb2.ConnectEventRequest,
    active_pb2.DisconnectEventRequest,
]


class AxServeObject:
    def __init__(
        self,
        channel: Union[grpc.Channel, str],
        channel_ready_timeout: Optional[int] = None,
        *,
        thread_constructor: Optional[Callable[..., Thread]] = None,
        thread_pool_executor: Optional[ThreadPoolExecutor] = None,
        start_event_loop: bool = True,
    ):
        try:
            self.__dict__["_properties_dict"] = {}
            self.__dict__["_methods_dict"] = {}
            self.__dict__["_events_dict"] = {}

            self._thread_local = threading.local()
            self._thread_local._handle_event_context_stack = []

            self._handle_event_response_queue: Optional[IterableQueue] = None
            self._handle_event_requests: Optional[Iterator[active_pb2.HandleEventRequest]] = None

            self._event_loop: Optional[AxServeEventLoop] = None
            self._event_loop_thread: Optional[Thread] = None
            self._event_loop_future: Optional[Future] = None
            self._event_loop_exception: Optional[Exception] = None

            self._server_process: Optional[AxServeServerProcess] = None
            self._channel: Optional[grpc.Channel] = None

            if channel_ready_timeout is None:
                channel_ready_timeout = 10

            if isinstance(channel, str):
                try:
                    pywintypes.IID(channel)
                except pywintypes.com_error:
                    address = channel
                    channel = grpc.insecure_channel(address)
                    self._channel = channel
                else:
                    clsid = channel
                    port = FindFreePort()
                    address = f"localhost:{port}"
                    server_process = AxServeServerProcess(clsid, address)
                    channel = grpc.insecure_channel(address)
                    self._server_process = server_process
                    self._channel = channel

            grpc.channel_ready_future(channel).result(timeout=channel_ready_timeout)

            self._stub = active_pb2_grpc.ActiveStub(channel)
            request = active_pb2.DescribeRequest()
            self._set_request_context(request)
            response = self._stub.Describe(request)
            response = typing.cast(active_pb2.DescribeResponse, response)

            self._properties_list: list[AxServeProperty] = []
            self._properties_dict: dict[str, AxServeProperty] = {}
            self._methods_list: list[AxServeMethod] = []
            self._methods_dict: dict[str, AxServeMethod] = {}
            self._events_list: list[AxServeEvent] = []
            self._events_dict: dict[str, AxServeEvent] = {}

            for info in response.properties:
                prop = AxServeProperty(self, info)
                self._properties_list.append(prop)
                self._properties_dict[info.name] = prop
            for info in response.methods:
                method = AxServeMethod(self, info)
                self._methods_list.append(method)
                self._methods_dict[info.name] = method
                if info.name in self._properties_dict:
                    self._properties_dict[info.name]._method = method
                else:
                    setattr(self, info.name, method)
            for info in response.events:
                event = AxServeEvent(self, info)
                self._events_list.append(event)
                self._events_dict[info.name] = event
                setattr(self, info.name, event)

            self._handle_event_response_queue = IterableQueue()
            self._handle_event_requests = self._stub.HandleEvent(self._handle_event_response_queue)
            self._event_loop = AxServeEventLoop(self)

            if not start_event_loop:
                pass
            elif thread_pool_executor:
                self._event_loop_future = thread_pool_executor.submit(self._event_loop_exec_target)
            else:
                if not thread_constructor:
                    thread_constructor = threading.Thread
                self._event_loop_thread = thread_constructor(target=self._event_loop_exec_target, daemon=True)
                self._event_loop_thread.start()
        except Exception:
            self.close()
            raise
        else:
            atexit.register(self.close)

    def __getattr__(self, name):
        if name in self._properties_dict:
            return self._properties_dict[name].__get__(self)
        return super().__getattribute__(name)

    def __setattr__(self, name, value):
        if name in self._properties_dict:
            return self._properties_dict[name].__set__(self, value)
        return super().__setattr__(name, value)

    def __dir__(self):
        props = list(self._properties_dict.keys())
        attrs = super().__dir__()
        return props + attrs

    def _get_handle_event_context_stack(self) -> list[active_pb2.HandleEventRequest]:
        if not hasattr(self._thread_local, "_handle_event_context_stack"):
            self._thread_local._handle_event_context_stack = []
        return self._thread_local._handle_event_context_stack

    def _set_request_context(self, request: AxServeCommonRequest) -> AxServeCommonRequest:
        event_context_stack = self._get_handle_event_context_stack()
        if event_context_stack:
            callback_event_index = event_context_stack[-1].index
            request.request_context = active_pb2.RequestContext.EVENT_CALLBACK
            request.callback_event_index = callback_event_index
        return request

    def _event_loop_exec_target(self):
        try:
            self._event_loop.exec()
        except grpc.RpcError as exc:
            self._event_loop_exception = exc
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
            self._event_loop_exception = exc
            self = None
            raise exc

    def event_loop(self) -> AxServeEventLoop:
        return self._event_loop

    def close(self, timeout: Optional[float] = None):
        if self._event_loop:
            self._event_loop.exit()
        if self._event_loop_thread:
            self._event_loop_thread.join(timeout=timeout)
        if self._event_loop_future:
            self._event_loop_future.result(timeout=timeout)
        if self._channel:
            self._channel.close()
        if self._server_process:
            self._server_process.terminate()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
