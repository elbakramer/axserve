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

#include "server_log_widget.h"

ServerLogWidget::ServerLogWidget(QTextEdit *log, QWidget *parent)
    : QWidget(parent),
      m_logEdit(log) {
  if (m_logEdit == nullptr) {
    m_logEdit = new QTextEdit(this);
  }
  m_logEdit->setReadOnly(true);
  m_clearButton = new QPushButton(tr("Clear"), this);
  connect(m_clearButton, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
  m_layout = new QVBoxLayout(this);
  m_layout->addWidget(m_logEdit);
  m_layout->addWidget(m_clearButton);
}

void ServerLogWidget::setLogEdit(QTextEdit *edit) {
  m_layout->removeWidget(m_logEdit);
  m_logEdit->deleteLater();
  m_logEdit = edit;
  m_layout->insertWidget(0, m_logEdit);
}
