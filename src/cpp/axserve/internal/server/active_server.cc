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

#include "active_server.h"

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

using grpc::Server;
using grpc::ServerBuilder;

ActiveServer::ActiveServer(QObject *parent)
    : QObject(parent) {
  m_control = std::make_unique<QAxWidget>();
}

ActiveServer::ActiveServer(const QString &control, QObject *parent)
    : ActiveServer(parent) {
  m_control->setControl(control);
}

ActiveServer::ActiveServer(
    const QString &control, const QStringList &addressList, QObject *parent
)
    : ActiveServer(control, parent) {
  for (const auto &address : addressList) {
    addListeningPort(address);
  }
}

ActiveServer::ActiveServer(
    const QString &control, const QString &address, QObject *parent
)
    : ActiveServer(control, parent) {
  addListeningPort(address);
}

ActiveServer::~ActiveServer() { shutdown(); }

void ActiveServer::addListeningPort(const QString &address_uri) {
  m_server_builder.AddListeningPort(
      address_uri.toStdString(), grpc::InsecureServerCredentials()
  );
}

bool ActiveServer::isRunning() { return m_server.get() != nullptr; }

bool ActiveServer::start() {
  shutdown();
  m_server = std::unique_ptr<Server>(m_server_builder.BuildAndStart());
  bool is_running = m_server.get() != nullptr;
  return is_running;
}

bool ActiveServer::shutdown() {
  bool is_running = m_server.get() != nullptr;
  if (is_running) {
    m_server->Shutdown();
  }
  m_server.reset();
  return is_running;
}

ulong ActiveServer::classContext() const { return m_control->classContext(); }

QString ActiveServer::control() const { return m_control->control(); }

void ActiveServer::resetControl() { m_control->resetControl(); }

void ActiveServer::setClassContext(ulong classContext) {
  m_control->setClassContext(classContext);
}

bool ActiveServer::setControl(const QString &c) {
  bool result = m_control->setControl(c);
  if (result) {
    m_processor = new ActiveServiceRequestProcessor(m_control.get(), this);
    m_service = std::make_unique<ActiveServiceImpl>(m_processor);
    m_server_builder.RegisterService(m_service.get());
  }
  return result;
}
