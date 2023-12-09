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

#include "show_message.h"

#include <qcoreapplication.h>
#include <qlatin1stringview.h>
#include <qstring.h>
#include <qsystemdetection.h>
#include <qtconfigmacros.h>
#include <qvariant.h>
#include <stdio.h>
#include <stdlib.h>

#include <QMessageBox>
#include <QtLogging>

QT_BEGIN_NAMESPACE

void showMessage(const QString &message, MessageType type) {
  bool noGui = false;
  bool noConsole = false;
  QCoreApplication *app = QCoreApplication::instance();
  if (app) {
    noGui = app->property("noGui").toBool();
    noConsole = app->property("noConsole").toBool();
  }
  if (!noGui) {
    QString title;
    if (app)
      title = app->property("applicationDisplayName").toString();
    if (title.isEmpty())
      title = QCoreApplication::applicationName();
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.exec();
  }
  if (!noConsole) {
    fputs(qPrintable(message), stderr);
    qDebug();
  }
}

QT_END_NAMESPACE
