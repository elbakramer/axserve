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

#include "server.h"

#include <chrono>

#include <QFile>

#include <grpcpp/security/server_credentials.h>

#include "axserve/server/security/api_key_auth_processor.h"

Server::Server(QObject *parent)
    : QObject(parent) {
  initialize();
}

Server::~Server() { shutdown(); }

void Server::initialize() {
  m_service.reset(new Service());
  m_serverBuilder.reset(new grpc::ServerBuilder());
  m_serverBuilder->RegisterService(m_service.get());
  m_server.reset();
}

bool Server::addControl(const QString &classId) {
  return m_service->addControl(classId);
}

void Server::addListeningPort(const QString &addressUri) {
  m_serverBuilder->AddListeningPort(
      addressUri.toStdString(), grpc::InsecureServerCredentials()
  );
}

bool Server::isRunning() { return m_server.get() != nullptr; }

bool Server::start(const ServerConfig &config) {
  shutdown();
  emit statusChanged(ServerStatus::Starting);
  if (!config.addressUri.isEmpty()) {
    auto serverCredentials = grpc::InsecureServerCredentials();
    if (!config.sslRootCertFile.isEmpty() &&
        !config.sslPrivateKeyFile.isEmpty() &&
        !config.sslCertChainFile.isEmpty()) {
      grpc::SslServerCredentialsOptions sslOptions;
      grpc::SslServerCredentialsOptions::PemKeyCertPair pemKeyCertPair;
      {
        QFile file(config.sslRootCertFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          emit statusChanged(ServerStatus::Error);
          return false;
        }
        sslOptions.pem_root_certs = file.readAll().toStdString();
      }
      {
        QFile file(config.sslPrivateKeyFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          emit statusChanged(ServerStatus::Error);
          return false;
        }
        pemKeyCertPair.private_key = file.readAll().toStdString();
      }
      {
        QFile file(config.sslCertChainFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          emit statusChanged(ServerStatus::Error);
          return false;
        }
        pemKeyCertPair.cert_chain = file.readAll().toStdString();
      }
      sslOptions.pem_key_cert_pairs.push_back(pemKeyCertPair);
      if (config.sslClientCertRequestType ==
          ServerSslClientCertRequestType::DontRequest) {
        sslOptions.client_certificate_request =
            GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE;
      } else if (config.sslClientCertRequestType ==
                 ServerSslClientCertRequestType::RequestButDontVerify) {
        sslOptions.client_certificate_request =
            GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_BUT_DONT_VERIFY;
      } else if (config.sslClientCertRequestType ==
                 ServerSslClientCertRequestType::RequestAndVerify) {
        sslOptions.client_certificate_request =
            GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_AND_VERIFY;
      } else if (config.sslClientCertRequestType ==
                 ServerSslClientCertRequestType::RequireButDontVerify) {
        sslOptions.client_certificate_request =
            GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_BUT_DONT_VERIFY;
      } else if (config.sslClientCertRequestType ==
                 ServerSslClientCertRequestType::RequireAndVerify) {
        sslOptions.client_certificate_request =
            GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY;
      }
      serverCredentials = grpc::SslServerCredentials(sslOptions);
    }
    if (!config.authApiKeyFile.isEmpty()) {
      std::unordered_set<std::string> authApiKeys;
      {
        QFile file(config.authApiKeyFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          emit statusChanged(ServerStatus::Error);
          return false;
        }
        std::string authApiKey = file.readAll().toStdString();
        authApiKeys.insert(authApiKey);
      }
      std::shared_ptr<ApiKeyAuthProcessor> authProcessor =
          std::make_shared<ApiKeyAuthProcessor>(authApiKeys);
      serverCredentials->SetAuthMetadataProcessor(authProcessor);
    }
    m_serverBuilder->AddListeningPort(
        config.addressUri.toStdString(), serverCredentials
    );
  }
  m_server = m_serverBuilder->BuildAndStart();
  bool is_running = isRunning();
  if (is_running) {
    emit statusChanged(ServerStatus::Running);
  } else {
    emit statusChanged(ServerStatus::Error);
  }
  return is_running;
}

bool Server::shutdown(double timeout) {
  bool is_running = isRunning();
  if (is_running) {
    if (timeout >= 0.0) {
      using namespace std::chrono;
      emit statusChanged(ServerStatus::Stopping);
      auto timeout_duration =
          duration_cast<system_clock::duration>(duration<double>(timeout));
      auto deadline = system_clock::now() + timeout_duration;
      m_server->Shutdown(deadline);
    } else {
      m_server->Shutdown();
    }
  }
  initialize();
  if (is_running) {
    emit statusChanged(ServerStatus::Stopped);
  }
  return is_running;
}
