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

#ifndef SERVER_WIDGET_H
#define SERVER_WIDGET_H

#include <QString>
#include <QTextEdit>
#include <QWidget>

#include "axserve/app/model/parsed_config.h"
#include "axserve/app/model/server_config.h"

#include "server_config_widget.h"
#include "server_log_widget.h"
#include "server_status_widget.h"

class ServerWidget : public QWidget {
  Q_OBJECT

public:
  ServerWidget(QWidget *parent = nullptr);

private:
  ServerConfigWidget *m_configWidget;
  ServerStatusWidget *m_statusWidget;
  ServerLogWidget *m_logWidget;

signals:
  void startRequested(const ServerConfig &config);
  void shutdownRequested();

public slots:
  void setStatus(ServerStatus status);
  void setParsedConfig(ParsedConfig config);

public:
  void setLogEdit(QTextEdit *edit);
  ServerConfig getServerConfig();
};

#endif // SERVER_WIDGET_H
