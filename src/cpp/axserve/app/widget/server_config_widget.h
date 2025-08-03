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

#ifndef SERVER_CONFIG_WIDGET_H
#define SERVER_CONFIG_WIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QValidator>
#include <QWidget>

#include "axserve/app/model/parsed_config.h"
#include "axserve/app/model/server_config.h"
#include "axserve/common/validator/address_uri_validator.h"
#include "axserve/server/server.h"

class ServerConfigWidget : public QWidget {
  Q_OBJECT

public:
  ServerConfigWidget(QWidget *parent = nullptr);

private:
  QString m_startText;
  QString m_startingText;
  QString m_shutdownText;
  QString m_shuttingDownText;

  QLabel *m_addressUriLabel;
  QLineEdit *m_addressUriEdit;
  QValidator *m_addressUriValidator;
  QPushButton *m_startOrShutdownButton;

  ServerStatus m_currentStatus;
  ParsedConfig m_parsedConfig;

  bool isAcceptableAddress(const QString &addressUri);
  ServerConfig collectServerConfig();
  void updateButtonText();

private slots:
  void handleButtonClick();

public slots:
  void setStatus(ServerStatus status);
  void setParsedConfig(ParsedConfig config);

signals:
  void startRequested(const ServerConfig &config);
  void shutdownRequested();

public:
  ServerConfig getServerConfig();
};

#endif // SERVER_CONFIG_WIDGET_H
