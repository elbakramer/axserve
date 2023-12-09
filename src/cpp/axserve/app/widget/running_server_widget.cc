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

#include "running_server_widget.h"

#include <Qt>
#include <QtDebug>
#include <QtLogging>

#include <QAction>
#include <QAxWidget>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPalette>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "axserve/common/logging/message_handlers_manager.h"
#include "axserve/common/logging/text_edit_message_appender.h"
#include "axserve/internal/server/active_server.h"

RunningServerWidget::RunningServerWidget(
    const QString &control, const QString &address, QWidget *parent,
    Qt::WindowFlags f
)
    : QWidget(parent, f) {

  // create server group box
  QGroupBox *serverGroupBox = new QGroupBox(this);
  QFormLayout *serverFormLayout = new QFormLayout(serverGroupBox);
  QString serverGroupBoxTitle = tr("Server");
  serverGroupBox->setTitle(serverGroupBoxTitle);
  serverGroupBox->setLayout(serverFormLayout);

  // create class id line edit
  QLabel *classIdLabel = new QLabel(serverGroupBox);
  QLineEdit *classIdLineEdit = new QLineEdit(control, serverGroupBox);
  QString classIdLabelText = tr("&CLSID:");
  QPalette classIdLineEditPalette = classIdLineEdit->palette();
  classIdLineEditPalette.setColor(
      QPalette::Base,
      classIdLineEditPalette.color(QPalette::Active, QPalette::Window)
  );
  classIdLabel->setText(classIdLabelText);
  classIdLabel->setBuddy(classIdLineEdit);
  classIdLineEdit->setPalette(classIdLineEditPalette);
  classIdLineEdit->setReadOnly(true);
  classIdLineEdit->setFrame(false);

  // create address uri line edit
  QLabel *addressUriLabel = new QLabel(serverGroupBox);
  QLineEdit *addressUriLineEdit = new QLineEdit(address, serverGroupBox);
  QString addressUriLabelText = tr("&Address URI:");
  QPalette addressUriLineEditPalette = addressUriLineEdit->palette();
  addressUriLineEditPalette.setColor(
      QPalette::Base,
      addressUriLineEditPalette.color(QPalette::Active, QPalette::Window)
  );
  addressUriLabel->setText(addressUriLabelText);
  addressUriLabel->setBuddy(addressUriLineEdit);
  addressUriLineEdit->setPalette(addressUriLineEditPalette);
  addressUriLineEdit->setReadOnly(true);
  addressUriLineEdit->setFrame(false);

  // add rows to server group box
  serverFormLayout->addRow(classIdLabel, classIdLineEdit);
  serverFormLayout->addRow(addressUriLabel, addressUriLineEdit);

  // create console group box
  QGroupBox *consoleGroupBox = new QGroupBox(this);
  QVBoxLayout *consoleVboxLayout = new QVBoxLayout(consoleGroupBox);
  consoleGroupBox->setTitle(tr("Log"));
  consoleGroupBox->setLayout(consoleVboxLayout);

  // create console text edit
  QPlainTextEdit *consoleTextEdit = new QPlainTextEdit(consoleGroupBox);
  QPalette consoleTextEditPalette = consoleTextEdit->palette();
  consoleTextEditPalette.setColor(
      QPalette::Base,
      consoleTextEditPalette.color(QPalette::Active, QPalette::Window)
  );
  consoleTextEdit->setReadOnly(true);
  consoleTextEdit->setPalette(consoleTextEditPalette);
  consoleTextEdit->setFrameShape(QFrame::Shape::NoFrame);
  consoleTextEdit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
  consoleTextEdit->setMaximumBlockCount(10000);
  consoleTextEdit->setCenterOnScroll(false);

  // add widgets to console group box
  consoleVboxLayout->addWidget(consoleTextEdit);

  // create main layout
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(serverGroupBox);
  mainLayout->addWidget(consoleGroupBox);
  setLayout(mainLayout);

  // create log appender
  m_edit = consoleTextEdit;
  m_appender = QSharedPointer<PlainTextEditMessageAppender>::create(m_edit);
  MessageHandlersManager::instance()->registerHandler(m_appender);

  // create server
  m_server = new ActiveServer(this);

  // initialize members
  m_isReady = false;
  m_failedReason = FailedReason::NONE;

  // set control
  if (!m_server->setControl(control)) {
    m_failedReason = FailedReason::CONTROL;
    return;
  }

  // add listening port and start server
  m_server->addListeningPort(address);
  if (!m_server->start()) {
    m_failedReason = FailedReason::SERVER;
    return;
  }

  // check ready
  m_isReady = true;
  qDebug() << "Server is ready";
}

bool RunningServerWidget::isReady() const { return m_isReady; }

RunningServerWidget::FailedReason RunningServerWidget::failedReason() const {
  return m_failedReason;
}
