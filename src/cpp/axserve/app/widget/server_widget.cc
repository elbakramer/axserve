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

#include "server_widget.h"

#include <QGroupBox>
#include <QSizePolicy>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <Qt>

ServerWidget::ServerWidget(QWidget *parent)
    : QWidget(parent) {
  QTextEdit *edit = new QTextEdit(this);

  QVBoxLayout *layout = new QVBoxLayout(this);
  QSplitter *splitter = new QSplitter(Qt::Vertical, this);

  m_configWidget = new ServerConfigWidget(this);
  m_statusWidget = new ServerStatusWidget(this);
  m_logWidget = new ServerLogWidget(edit, this);

  QGroupBox *configGroup = new QGroupBox(tr("Server Configuration"), this);
  QVBoxLayout *configLayout = new QVBoxLayout(configGroup);
  configLayout->addWidget(m_configWidget);
  configGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  splitter->addWidget(configGroup);
  splitter->setStretchFactor(0, 0);

  QGroupBox *statusGroup = new QGroupBox(tr("Server Status"), this);
  QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
  statusLayout->addWidget(m_statusWidget);
  statusGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  splitter->addWidget(statusGroup);
  splitter->setStretchFactor(1, 0);

  QGroupBox *logGroup = new QGroupBox(tr("Server Log"), this);
  QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
  logLayout->addWidget(m_logWidget);
  logGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  splitter->addWidget(logGroup);
  splitter->setStretchFactor(2, 1);

  layout->addWidget(splitter);

  connect(
      m_configWidget, &ServerConfigWidget::startRequested, this,
      &ServerWidget::startRequested
  );
  connect(
      m_configWidget, &ServerConfigWidget::shutdownRequested, this,
      &ServerWidget::shutdownRequested
  );
}

void ServerWidget::setStatus(ServerStatus status) {
  m_configWidget->setStatus(status);
  m_statusWidget->setStatus(status);
}

void ServerWidget::setParsedConfig(ParsedConfig config) {
  m_configWidget->setParsedConfig(config);
}

void ServerWidget::setLogEdit(QTextEdit *edit) {
  m_logWidget->setLogEdit(edit);
}

ServerConfig ServerWidget::getServerConfig() {
  return m_configWidget->getServerConfig();
}
