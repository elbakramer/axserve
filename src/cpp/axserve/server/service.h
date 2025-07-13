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

#ifndef SERVICE_H
#define SERVICE_H

#include <QSharedPointer>
#include <QString>

#include "active.grpc.pb.h"

#include "executor.h"

using grpc::CallbackServerContext;
using grpc::ServerBidiReactor;
using grpc::ServerUnaryReactor;

using namespace axserve;

class Service : public Active::CallbackService {

private:
  QSharedPointer<Executor> m_executor;

public:
  Service();

public:
  bool addControl(const QString &classId);

public:
  ServerUnaryReactor *Create(
      CallbackServerContext *context, const CreateRequest *request,
      CreateResponse *response
  ) override;
  ServerUnaryReactor *Refer(
      CallbackServerContext *context, const ReferRequest *request,
      ReferResponse *response
  ) override;
  ServerUnaryReactor *Release(
      CallbackServerContext *context, const ReleaseRequest *request,
      ReleaseResponse *response
  ) override;
  ServerUnaryReactor *Destroy(
      CallbackServerContext *context, const DestroyRequest *request,
      DestroyResponse *response
  ) override;
  ServerUnaryReactor *List(
      CallbackServerContext *context, const ListRequest *request,
      ListResponse *response
  ) override;

public:
  ServerUnaryReactor *Describe(
      CallbackServerContext *context, const DescribeRequest *request,
      DescribeResponse *response
  ) override;
  ServerUnaryReactor *GetProperty(
      CallbackServerContext *context, const GetPropertyRequest *request,
      GetPropertyResponse *response
  ) override;
  ServerUnaryReactor *SetProperty(
      CallbackServerContext *context, const SetPropertyRequest *request,
      SetPropertyResponse *response
  ) override;
  ServerUnaryReactor *InvokeMethod(
      CallbackServerContext *context, const InvokeMethodRequest *request,
      InvokeMethodResponse *response
  ) override;
  ServerUnaryReactor *ConnectEvent(
      CallbackServerContext *context, const ConnectEventRequest *request,
      ConnectEventResponse *response
  ) override;
  ServerUnaryReactor *DisconnectEvent(
      CallbackServerContext *context, const DisconnectEventRequest *request,
      DisconnectEventResponse *response
  ) override;

public:
  ServerBidiReactor<HandleEventResponse, HandleEventRequest> *
  HandleEvent(CallbackServerContext *context) override;
};

#endif // SERVICE_H
