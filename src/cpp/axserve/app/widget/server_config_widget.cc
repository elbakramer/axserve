// Copyright 2025 Yunseong Hwang
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

#include "server_config_widget.h"

#include <QVBoxLayout>

ServerConfigWidget::ServerConfigWidget(QWidget *parent)
    : QWidget(parent),
      m_currentStatus(ServerStatus::Stopped) {
  m_startText = tr("Start");
  m_startingText = tr("Starting...");
  m_shutdownText = tr("Shutdown");
  m_shuttingDownText = tr("Shutting down...");

  QString addressLabelText = tr("Address URI:");
  QString addressEditPlaceholder = "localhost:9501";
  QString addressEditToolTip =
      tr("The address to try to bind to the server in URI form.\n"
         "If the scheme name is omitted, \"dns:///\" is assumed.\n"
         "To bind to any address, please use IPv6 any, i.e., [::]:<port>, "
         "which also accepts IPv4 connections.\n"
         "Valid values include dns:///localhost:1234, 192.168.1.1:31416, "
         "dns:///[::1]:27182, etc.");

  m_addressUriLabel = new QLabel(addressLabelText, this);
  m_addressUriEdit = new QLineEdit(this);
  m_addressUriEdit->setPlaceholderText(addressEditPlaceholder);
  m_addressUriEdit->setToolTip(addressEditToolTip);
  m_addressUriValidator = new AddressURIValidator(this);
  m_addressUriEdit->setValidator(m_addressUriValidator);
  m_startOrShutdownButton = new QPushButton(m_startText, this);

  connect(
      m_startOrShutdownButton, &QPushButton::clicked, this,
      &ServerConfigWidget::handleButtonClick
  );

  QVBoxLayout *layout = new QVBoxLayout(this);

  layout->addWidget(m_addressUriLabel);
  layout->addWidget(m_addressUriEdit);
  layout->addWidget(m_startOrShutdownButton);
}

bool ServerConfigWidget::isAcceptableAddress(const QString &addressUri) {
  QString addressUriCopy = addressUri;
  int pos = 0;
  return m_addressUriValidator->validate(addressUriCopy, pos) ==
      QValidator::Acceptable;
}

ServerConfig ServerConfigWidget::collectServerConfig() {
  ServerConfig config;
  QString address = m_addressUriEdit->text();
  if (address.isEmpty()) {
    address = m_addressUriEdit->placeholderText();
    m_addressUriEdit->setText(address);
  }
  if (isAcceptableAddress(address)) {
    config.addressUri = address;
  }
  return config;
}

void ServerConfigWidget::updateButtonText() {
  switch (m_currentStatus) {
  case ServerStatus::Stopped:
    m_addressUriEdit->setEnabled(true);
    m_startOrShutdownButton->setText(m_startText);
    m_startOrShutdownButton->setEnabled(true);
    break;
  case ServerStatus::Starting:
    m_addressUriEdit->setEnabled(false);
    m_startOrShutdownButton->setEnabled(false);
    m_startOrShutdownButton->setText(m_startingText);
    break;
  case ServerStatus::Running:
    m_startOrShutdownButton->setText(m_shutdownText);
    m_startOrShutdownButton->setEnabled(true);
    break;
  case ServerStatus::Stopping:
    m_startOrShutdownButton->setEnabled(false);
    m_startOrShutdownButton->setText(m_shuttingDownText);
    break;
  case ServerStatus::Error:
    m_addressUriEdit->setEnabled(true);
    m_startOrShutdownButton->setText(m_startText);
    m_startOrShutdownButton->setEnabled(true);
    break;
  }
}

void ServerConfigWidget::handleButtonClick() {
  if (m_currentStatus == ServerStatus::Stopped) {
    ServerConfig config = getServerConfig();
    if (!config.addressUri.isEmpty()) {
      emit startRequested(config);
    }
  } else if (m_currentStatus == ServerStatus::Running) {
    emit shutdownRequested();
  }
}

void ServerConfigWidget::setStatus(ServerStatus status) {
  m_currentStatus = status;
  updateButtonText();
}

void ServerConfigWidget::setParsedConfig(ParsedConfig config) {
  m_parsedConfig = std::move(config);
  if (!m_parsedConfig.addressUri.empty()) {
    m_addressUriEdit->setText(
        QString::fromStdString(m_parsedConfig.addressUri)
    );
  }
}

ServerConfig ServerConfigWidget::getServerConfig() {
  ServerConfig config = collectServerConfig();

  config.sslRootCertFile =
      QString::fromStdString(m_parsedConfig.sslRootCertFile);
  config.sslPrivateKeyFile =
      QString::fromStdString(m_parsedConfig.sslPrivateKeyFile);
  config.sslCertChainFile =
      QString::fromStdString(m_parsedConfig.sslCertChainFile);

  if (m_parsedConfig.sslRootCertFile == "" ||
      m_parsedConfig.sslRootCertFile == "dont-request") {
    config.sslClientCertRequestType =
        ServerSslClientCertRequestType::DontRequest;
  } else if (m_parsedConfig.sslRootCertFile == "request-but-dont-verify") {
    config.sslClientCertRequestType =
        ServerSslClientCertRequestType::RequestButDontVerify;
  } else if (m_parsedConfig.sslRootCertFile == "request-and-verify") {
    config.sslClientCertRequestType =
        ServerSslClientCertRequestType::RequireAndVerify;
  } else if (m_parsedConfig.sslRootCertFile == "require-but-dont-verify") {
    config.sslClientCertRequestType =
        ServerSslClientCertRequestType::RequireButDontVerify;
  } else if (m_parsedConfig.sslRootCertFile == "require-and-verify") {
    config.sslClientCertRequestType =
        ServerSslClientCertRequestType::RequireAndVerify;
  }

  config.authApiKeyFile = QString::fromStdString(m_parsedConfig.authApiKeyFile);

  return config;
}
