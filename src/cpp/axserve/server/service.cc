// Copyright 2023 Yunseong Hwang
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "service.h"

#include "inbound_reactor.h"
#include "outbound_reactor.h"

Service::Service() { m_executor = Executor::create(); }

bool Service::addControl(const QString &classId) {
  return m_executor->addControl(classId);
}

ServerUnaryReactor *Service::Create(
    CallbackServerContext *context, const CreateRequest *request,
    CreateResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::Refer(
    CallbackServerContext *context, const ReferRequest *request,
    ReferResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::Release(
    CallbackServerContext *context, const ReleaseRequest *request,
    ReleaseResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::Destroy(
    CallbackServerContext *context, const DestroyRequest *request,
    DestroyResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::List(
    CallbackServerContext *context, const ListRequest *request,
    ListResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}

ServerUnaryReactor *Service::Describe(
    CallbackServerContext *context, const DescribeRequest *request,
    DescribeResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::GetProperty(
    CallbackServerContext *context, const GetPropertyRequest *request,
    GetPropertyResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::SetProperty(
    CallbackServerContext *context, const SetPropertyRequest *request,
    SetPropertyResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::InvokeMethod(
    CallbackServerContext *context, const InvokeMethodRequest *request,
    InvokeMethodResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::ConnectEvent(
    CallbackServerContext *context, const ConnectEventRequest *request,
    ConnectEventResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}
ServerUnaryReactor *Service::DisconnectEvent(
    CallbackServerContext *context, const DisconnectEventRequest *request,
    DisconnectEventResponse *response
) {
  return new InboundReactor(m_executor, context, request, response);
}

ServerBidiReactor<HandleEventResponse, HandleEventRequest> *
Service::HandleEvent(CallbackServerContext *context) {
  QSharedPointer<OutboundReactor> reactor =
      OutboundReactor::create(m_executor, context);
  return reactor.get();
}
