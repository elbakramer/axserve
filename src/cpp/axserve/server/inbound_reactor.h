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
 * SPDX-FileCopyrightText: 2025 Yunseong Hwang
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef INBOUND_REACTOR_H
#define INBOUND_REACTOR_H

#include <QSharedPointer>

#include "active.grpc.pb.h"

using grpc::CallbackServerContext;
using grpc::ServerUnaryReactor;
using grpc::Status;

using namespace axserve;

class Executor;
class InboundItem;

class InboundReactor : public ServerUnaryReactor {
private:
  QSharedPointer<InboundItem> m_item;

public:
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const CreateRequest *request, CreateResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const ReferRequest *request, ReferResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const ReleaseRequest *request, ReleaseResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const DestroyRequest *request, DestroyResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const ListRequest *request, ListResponse *response
  );

public:
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const DescribeRequest *request, DescribeResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const GetPropertyRequest *request, GetPropertyResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const SetPropertyRequest *request, SetPropertyResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const InvokeMethodRequest *request, InvokeMethodResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const ConnectEventRequest *request, ConnectEventResponse *response
  );
  InboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context,
      const DisconnectEventRequest *request, DisconnectEventResponse *response
  );

public:
  void OnDone() override;
  void OnCancel() override;
};

#endif // INBOUND_REACTOR_H
