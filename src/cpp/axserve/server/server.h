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

#ifndef SERVER_H
#define SERVER_H

#include <memory>

#include <QObject>
#include <QString>

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "server_status.h"
#include "service.h"

#include "axserve/app/model/parsed_config.h"
#include "axserve/app/model/server_config.h"

class Server : public QObject {
  Q_OBJECT

private:
  std::unique_ptr<Service> m_service;

  std::unique_ptr<grpc::ServerBuilder> m_serverBuilder;
  std::unique_ptr<grpc::Server> m_server;

private:
  void initialize();

public:
  explicit Server(QObject *parent = nullptr);
  virtual ~Server() override;

  bool addControl(const QString &classId);
  void addListeningPort(const QString &addressUri);

  bool isRunning();

signals:
  void statusChanged(ServerStatus status);

public slots:
  bool start(const ServerConfig &config = {});
  bool shutdown(double timeout = 15.0);
};

#endif // SERVER_H
