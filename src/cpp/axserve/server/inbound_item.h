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

#ifndef INBOUND_ITEM_H
#define INBOUND_ITEM_H

#include <QFuture>
#include <QPromise>
#include <QString>
#include <QUuid>

#include "active.grpc.pb.h"

using grpc::CallbackServerContext;
using grpc::ServerUnaryReactor;

using namespace axserve;

class InboundItem {
public:
  enum Type {
    CREATE,
    REFER,
    RELEASE,
    DESTROY,
    LIST,
    DESCRIBE,
    GET_PROPERTY,
    SET_PROPERTY,
    INVOKE_METHOD,
    CONNECT_EVENT,
    DISCONNECT_EVENT,
  };

protected:
  InboundItem::Type m_type;
  CallbackServerContext *m_context;
  ServerUnaryReactor *m_reactor;
  QPromise<void> m_promise;
  QFuture<void> m_future;

public:
  InboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor
  )
      : m_type(type),
        m_context(context),
        m_reactor(reactor) {
    m_future = m_promise.future();
  }

  virtual ~InboundItem() = default;

  InboundItem::Type type() const { return m_type; }

  CallbackServerContext *call() const { return m_context; }
  ServerUnaryReactor *reactor() const { return m_reactor; }

  QString peer() const { return QString::fromStdString(m_context->peer()); }

  bool started() const { return m_future.isStarted(); }
  bool running() const { return m_future.isRunning(); }
  bool finished() const { return m_future.isFinished(); }
  bool canceled() const { return m_promise.isCanceled(); }

  void start() { m_promise.start(); }
  void finish() { m_promise.finish(); }
  void cancel() { m_future.cancel(); }

  virtual const Context &context() const = 0;
  virtual const ContextType &contextType() const = 0;
  virtual const QUuid &contextId() const = 0;
};

template <typename Request, typename Response>
class FullInboundItem : public InboundItem {
protected:
  const Request *m_request;
  Response *m_response;

  ContextType m_contextType;
  QUuid m_contextId;

public:
  FullInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const Request *request, Response *response
  )
      : InboundItem(type, context, reactor),
        m_request(request),
        m_response(response) {
    m_contextType = m_request->context().context_type();
    std::string id = m_request->context().context_info().id();
    if (!id.empty()) {
      m_contextId = QUuid::fromString(id);
    }
  }

  virtual ~FullInboundItem() = default;

  const Request *request() const { return m_request; }
  Response *response() const { return m_response; }

  const Context &context() const override { return m_request->context(); }
  const ContextType &contextType() const override { return m_contextType; }
  const QUuid &contextId() const override { return m_contextId; }
};

class CreateInboundItem
    : public FullInboundItem<CreateRequest, CreateResponse> {
public:
  CreateInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const CreateRequest *request,
      CreateResponse *response
  )
      : FullInboundItem<CreateRequest, CreateResponse>(
            type, context, reactor, request, response
        ) {}
  CreateInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const CreateRequest *request, CreateResponse *response
  )
      : CreateInboundItem(
            InboundItem::Type::CREATE, context, reactor, request, response
        ) {}
};
class ReferInboundItem : public FullInboundItem<ReferRequest, ReferResponse> {
public:
  ReferInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const ReferRequest *request,
      ReferResponse *response
  )
      : FullInboundItem<ReferRequest, ReferResponse>(
            type, context, reactor, request, response
        ) {}
  ReferInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const ReferRequest *request, ReferResponse *response
  )
      : ReferInboundItem(
            InboundItem::Type::REFER, context, reactor, request, response
        ) {}
};
class ReleaseInboundItem
    : public FullInboundItem<ReleaseRequest, ReleaseResponse> {
public:
  ReleaseInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const ReleaseRequest *request,
      ReleaseResponse *response
  )
      : FullInboundItem<ReleaseRequest, ReleaseResponse>(
            type, context, reactor, request, response
        ) {}
  ReleaseInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const ReleaseRequest *request, ReleaseResponse *response
  )
      : ReleaseInboundItem(
            InboundItem::Type::RELEASE, context, reactor, request, response
        ) {}
};
class DestroyInboundItem
    : public FullInboundItem<DestroyRequest, DestroyResponse> {
public:
  DestroyInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const DestroyRequest *request,
      DestroyResponse *response
  )
      : FullInboundItem<DestroyRequest, DestroyResponse>(
            type, context, reactor, request, response
        ) {}
  DestroyInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const DestroyRequest *request, DestroyResponse *response
  )
      : DestroyInboundItem(
            InboundItem::Type::DESTROY, context, reactor, request, response
        ) {}
};
class ListInboundItem : public FullInboundItem<ListRequest, ListResponse> {
public:
  ListInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const ListRequest *request,
      ListResponse *response
  )
      : FullInboundItem<ListRequest, ListResponse>(
            type, context, reactor, request, response
        ) {}
  ListInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const ListRequest *request, ListResponse *response
  )
      : ListInboundItem(
            InboundItem::Type::LIST, context, reactor, request, response
        ) {}
};

class DescribeInboundItem
    : public FullInboundItem<DescribeRequest, DescribeResponse> {
public:
  DescribeInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const DescribeRequest *request,
      DescribeResponse *response
  )
      : FullInboundItem<DescribeRequest, DescribeResponse>(
            type, context, reactor, request, response
        ) {}
  DescribeInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const DescribeRequest *request, DescribeResponse *response
  )
      : DescribeInboundItem(
            InboundItem::Type::DESCRIBE, context, reactor, request, response
        ) {}
};
class GetPropertyInboundItem
    : public FullInboundItem<GetPropertyRequest, GetPropertyResponse> {
public:
  GetPropertyInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const GetPropertyRequest *request,
      GetPropertyResponse *response
  )
      : FullInboundItem<GetPropertyRequest, GetPropertyResponse>(
            type, context, reactor, request, response
        ) {}
  GetPropertyInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const GetPropertyRequest *request, GetPropertyResponse *response
  )
      : GetPropertyInboundItem(
            InboundItem::Type::GET_PROPERTY, context, reactor, request, response
        ) {}
};
class SetPropertyInboundItem
    : public FullInboundItem<SetPropertyRequest, SetPropertyResponse> {
public:
  SetPropertyInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const SetPropertyRequest *request,
      SetPropertyResponse *response
  )
      : FullInboundItem<SetPropertyRequest, SetPropertyResponse>(
            type, context, reactor, request, response
        ) {}
  SetPropertyInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const SetPropertyRequest *request, SetPropertyResponse *response
  )
      : SetPropertyInboundItem(
            InboundItem::Type::SET_PROPERTY, context, reactor, request, response
        ) {}
};
class InvokeMethodInboundItem
    : public FullInboundItem<InvokeMethodRequest, InvokeMethodResponse> {
public:
  InvokeMethodInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const InvokeMethodRequest *request,
      InvokeMethodResponse *response
  )
      : FullInboundItem<InvokeMethodRequest, InvokeMethodResponse>(
            type, context, reactor, request, response
        ) {}
  InvokeMethodInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const InvokeMethodRequest *request, InvokeMethodResponse *response
  )
      : InvokeMethodInboundItem(
            InboundItem::Type::INVOKE_METHOD, context, reactor, request,
            response
        ) {}
};
class ConnectEventInboundItem
    : public FullInboundItem<ConnectEventRequest, ConnectEventResponse> {
public:
  ConnectEventInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const ConnectEventRequest *request,
      ConnectEventResponse *response
  )
      : FullInboundItem<ConnectEventRequest, ConnectEventResponse>(
            type, context, reactor, request, response
        ) {}
  ConnectEventInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const ConnectEventRequest *request, ConnectEventResponse *response
  )
      : ConnectEventInboundItem(
            InboundItem::Type::CONNECT_EVENT, context, reactor, request,
            response
        ) {}
};
class DisconnectEventInboundItem
    : public FullInboundItem<DisconnectEventRequest, DisconnectEventResponse> {
public:
  DisconnectEventInboundItem(
      InboundItem::Type type, CallbackServerContext *context,
      ServerUnaryReactor *reactor, const DisconnectEventRequest *request,
      DisconnectEventResponse *response
  )
      : FullInboundItem<DisconnectEventRequest, DisconnectEventResponse>(
            type, context, reactor, request, response
        ) {}
  DisconnectEventInboundItem(
      CallbackServerContext *context, ServerUnaryReactor *reactor,
      const DisconnectEventRequest *request, DisconnectEventResponse *response
  )
      : DisconnectEventInboundItem(
            InboundItem::Type::DISCONNECT_EVENT, context, reactor, request,
            response
        ) {}
};

#endif // INBOUND_ITEM_H
