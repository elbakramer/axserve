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

#include "server_status_widget.h"

#include <QFormLayout>

ServerStatusWidget::ServerStatusWidget(QWidget *parent)
    : QWidget(parent) {
  QFormLayout *layout = new QFormLayout(this);
  m_statusLabel = new QLabel("Stopped", this);
  layout->addRow("Status:", m_statusLabel);
}

void ServerStatusWidget::setStatus(ServerStatus status) {
  switch (status) {
  case ServerStatus::Stopped:
    m_statusLabel->setText("Stopped");
    break;
  case ServerStatus::Starting:
    m_statusLabel->setText("Starting...");
    break;
  case ServerStatus::Running:
    m_statusLabel->setText("Running");
    break;
  case ServerStatus::Stopping:
    m_statusLabel->setText("Stopping...");
    break;
  case ServerStatus::Error:
    m_statusLabel->setText("Error!");
    break;
  }
}
