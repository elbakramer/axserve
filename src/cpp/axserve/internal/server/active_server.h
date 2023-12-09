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

#ifndef ACTIVE_SERVER_H
#define ACTIVE_SERVER_H

#include <memory>

#include <QAxObjectInterface>
#include <QAxWidget>
#include <QObject>
#include <QString>

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "active_service_impl.h"
#include "active_service_request_processor.h"

using grpc::Server;
using grpc::ServerBuilder;

class ActiveServer : public QObject, public QAxObjectInterface {
  Q_OBJECT

private:
  std::unique_ptr<QAxWidget> m_control;

  ActiveServiceRequestProcessor *m_processor;
  std::unique_ptr<ActiveServiceImpl> m_service;

  ServerBuilder m_server_builder;
  std::unique_ptr<Server> m_server;

public:
  ActiveServer(QObject *parent = nullptr);
  ActiveServer(const QString &control, QObject *parent = nullptr);
  ActiveServer(
      const QString &control, const QString &address, QObject *parent = nullptr
  );
  ActiveServer(
      const QString &control, const QStringList &addressList,
      QObject *parent = nullptr
  );

  ~ActiveServer() override;

  // server
  void addListeningPort(const QString &address_uri);
  bool isRunning();

  // control
  ulong classContext() const override;
  QString control() const override;
  void resetControl() override;
  void setClassContext(ulong classContext) override;
  bool setControl(const QString &c) override;

public slots:
  bool start();
  bool shutdown();
};

#endif
