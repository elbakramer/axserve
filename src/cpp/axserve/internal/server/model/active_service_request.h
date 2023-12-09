/*
 * Copyright 2023 Yunseong Hwang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ACTIVE_SERVICE_REQUEST_H
#define ACTIVE_SERVICE_REQUEST_H

#include "active.grpc.pb.h"

using grpc::CallbackServerContext;
using grpc::ServerUnaryReactor;

class ActiveServiceRequest {
public:
  enum Type {
    DESCRIBE,
    GET_PROPERTY,
    SET_PROPERTY,
    INVOKE_METHOD,
    CONNECT_EVENT,
    DISCONNECT_EVENT,
  };

private:
  ActiveServiceRequest::Type m_type;

  CallbackServerContext *m_context;
  ServerUnaryReactor *m_reactor;

  RequestContext m_request_context;
  uint32_t m_callback_event_index;

public:
  ActiveServiceRequest(
      ActiveServiceRequest::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, RequestContext request_context,
      uint32_t callback_event_index
  )
      : m_type(type),
        m_context(context),
        m_reactor(reactor),
        m_request_context(request_context),
        m_callback_event_index(callback_event_index) {}

  ActiveServiceRequest::Type type() const { return m_type; }

  CallbackServerContext *context() const { return m_context; }
  ServerUnaryReactor *reactor() const { return m_reactor; }

  RequestContext request_context() const { return m_request_context; }
  uint32_t callback_event_index() const { return m_callback_event_index; }
};

template <class Request, class Response>
class FullActiveServiceRequest : public ActiveServiceRequest {
private:
  const Request *m_request;
  Response *m_response;

public:
  FullActiveServiceRequest(
      ActiveServiceRequest::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const Request *request, Response *response,
      RequestContext request_context, uint32_t callback_event_index
  )
      : ActiveServiceRequest(
            type, context, reactor, request_context, callback_event_index
        ),
        m_request(request),
        m_response(response) {}

  const Request *request() const { return m_request; }
  Response *response() const { return m_response; }
};

class DescribeActiveServiceRequest
    : public FullActiveServiceRequest<DescribeRequest, DescribeResponse> {
public:
  DescribeActiveServiceRequest(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const DescribeRequest *request, DescribeResponse *response
  )
      : FullActiveServiceRequest<DescribeRequest, DescribeResponse>(
            ActiveServiceRequest::Type::DESCRIBE, context, reactor, request,
            response, request->request_context(),
            request->callback_event_index()
        ) {}
};

class GetPropertyActiveServiceRequest
    : public FullActiveServiceRequest<GetPropertyRequest, GetPropertyResponse> {
public:
  GetPropertyActiveServiceRequest(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const GetPropertyRequest *request, GetPropertyResponse *response
  )
      : FullActiveServiceRequest<GetPropertyRequest, GetPropertyResponse>(
            ActiveServiceRequest::Type::GET_PROPERTY, context, reactor, request,
            response, request->request_context(),
            request->callback_event_index()
        ) {}
};

class SetPropertyActiveServiceRequest
    : public FullActiveServiceRequest<SetPropertyRequest, SetPropertyResponse> {
public:
  SetPropertyActiveServiceRequest(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const SetPropertyRequest *request, SetPropertyResponse *response
  )
      : FullActiveServiceRequest<SetPropertyRequest, SetPropertyResponse>(
            ActiveServiceRequest::Type::SET_PROPERTY, context, reactor, request,
            response, request->request_context(),
            request->callback_event_index()
        ) {}
};

class InvokeMethodActiveServiceRequest
    : public FullActiveServiceRequest<
          InvokeMethodRequest, InvokeMethodResponse> {
public:
  InvokeMethodActiveServiceRequest(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const InvokeMethodRequest *request, InvokeMethodResponse *response
  )
      : FullActiveServiceRequest<InvokeMethodRequest, InvokeMethodResponse>(
            ActiveServiceRequest::Type::INVOKE_METHOD, context, reactor,
            request, response, request->request_context(),
            request->callback_event_index()
        ) {}
};

class ConnectEventActiveServiceRequest
    : public FullActiveServiceRequest<
          ConnectEventRequest, ConnectEventResponse> {
public:
  ConnectEventActiveServiceRequest(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const ConnectEventRequest *request, ConnectEventResponse *response
  )
      : FullActiveServiceRequest<ConnectEventRequest, ConnectEventResponse>(
            ActiveServiceRequest::Type::CONNECT_EVENT, context, reactor,
            request, response, request->request_context(),
            request->callback_event_index()
        ) {}
};

class DisconnectEventActiveServiceRequest
    : public FullActiveServiceRequest<
          DisconnectEventRequest, DisconnectEventResponse> {
public:
  DisconnectEventActiveServiceRequest(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const DisconnectEventRequest *request, DisconnectEventResponse *response
  )
      : FullActiveServiceRequest<
            DisconnectEventRequest, DisconnectEventResponse>(
            ActiveServiceRequest::Type::DISCONNECT_EVENT, context, reactor,
            request, response, request->request_context(),
            request->callback_event_index()
        ) {}
};

#endif
