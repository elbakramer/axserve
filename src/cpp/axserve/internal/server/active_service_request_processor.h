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

#ifndef ACTIVE_SERVICE_REQUEST_HANDLER_H
#define ACTIVE_SERVICE_REQUEST_HANDLER_H

#include <QAxWidget>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>
#include <QWaitCondition>

#include "active_service_request_queue.h"
#include "control_extension.h"
#include "handle_event_reactor.h"
#include "handle_event_reactors_manager.h"

#include "active.grpc.pb.h"

using grpc::CallbackServerContext;
using grpc::ServerBidiReactor;
using grpc::ServerUnaryReactor;

class ActiveServiceRequestProcessor : public QObject {
  Q_OBJECT

private:
  QAxWidget *m_control;
  ControlExtension m_control_extension;

  QSharedPointer<ActiveServiceRequestQueue> m_default_context_request_queue;
  QList<QSharedPointer<ActiveServiceRequestQueue>>
      m_handle_context_request_queues;

  QSharedPointer<HandleEventReactorsManager> m_reactors_manager;

  QMutex m_event_context_mutex;
  QSharedPointer<QWaitCondition> m_event_context_condition;

  int m_increment_on_slot;

public:
  ActiveServiceRequestProcessor(QAxWidget *control, QObject *parent = nullptr);

  ServerUnaryReactor *create_unary_reactor(
      CallbackServerContext *context, const DescribeRequest *request,
      DescribeResponse *response
  );
  ServerUnaryReactor *create_unary_reactor(
      CallbackServerContext *context, const GetPropertyRequest *request,
      GetPropertyResponse *response
  );
  ServerUnaryReactor *create_unary_reactor(
      CallbackServerContext *context, const SetPropertyRequest *request,
      SetPropertyResponse *response
  );
  ServerUnaryReactor *create_unary_reactor(
      CallbackServerContext *context, const InvokeMethodRequest *request,
      InvokeMethodResponse *response
  );
  ServerUnaryReactor *create_unary_reactor(
      CallbackServerContext *context, const ConnectEventRequest *request,
      ConnectEventResponse *response
  );
  ServerUnaryReactor *create_unary_reactor(
      CallbackServerContext *context, const DisconnectEventRequest *request,
      DisconnectEventResponse *response
  );
  HandleEventReactor *create_handle_reactor(CallbackServerContext *context);

  void push_request(ActiveServiceRequest *request);
  void process_request(ActiveServiceRequest *request);

  void process_request(DescribeActiveServiceRequest *request);
  void process_request(GetPropertyActiveServiceRequest *request);
  void process_request(SetPropertyActiveServiceRequest *request);
  void process_request(InvokeMethodActiveServiceRequest *request);
  void process_request(ConnectEventActiveServiceRequest *request);
  void process_request(DisconnectEventActiveServiceRequest *request);

  void process_default_context_requests();
  void process_handle_context_requests(int index);

signals:
  void pushed();

private slots:
  void process();
  void slot(const QString &name, int argc, void *argv);
};

#endif
