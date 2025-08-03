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
// SPDX-FileCopyrightText: 2025 Yunseong Hwang
//
// SPDX-License-Identifier: Apache-2.0

#include "inbound_reactor.h"

#include "executor.h"
#include "inbound_item.h"

#include "axserve/common/logging/protobuf_message_logging.h"

InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const CreateRequest *request, CreateResponse *response
) {
  m_item = QSharedPointer<CreateInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const ReferRequest *request, ReferResponse *response
) {
  m_item = QSharedPointer<ReferInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const ReleaseRequest *request, ReleaseResponse *response
) {
  m_item = QSharedPointer<ReleaseInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const DestroyRequest *request, DestroyResponse *response
) {
  m_item = QSharedPointer<DestroyInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const ListRequest *request, ListResponse *response
) {
  m_item =
      QSharedPointer<ListInboundItem>::create(context, this, request, response);
  executor->schedule(m_item);
}

InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const DescribeRequest *request, DescribeResponse *response
) {
  m_item = QSharedPointer<DescribeInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const GetPropertyRequest *request, GetPropertyResponse *response
) {
  m_item = QSharedPointer<GetPropertyInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const SetPropertyRequest *request, SetPropertyResponse *response
) {
  m_item = QSharedPointer<SetPropertyInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const InvokeMethodRequest *request, InvokeMethodResponse *response
) {
  m_item = QSharedPointer<InvokeMethodInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const ConnectEventRequest *request, ConnectEventResponse *response
) {
  m_item = QSharedPointer<ConnectEventInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}
InboundReactor::InboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context,
    const DisconnectEventRequest *request, DisconnectEventResponse *response
) {
  m_item = QSharedPointer<DisconnectEventInboundItem>::create(
      context, this, request, response
  );
  executor->schedule(m_item);
}

void InboundReactor::OnDone() { delete this; }

void InboundReactor::OnCancel() {
  m_item->cancel();
  if (!m_item->started()) {
    Finish(Status::CANCELLED);
  }
}
