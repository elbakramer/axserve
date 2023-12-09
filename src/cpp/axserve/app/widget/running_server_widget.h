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

#ifndef RUNNING_SERVER_WIDGET_H
#define RUNNING_SERVER_WIDGET_H

#include <Qt>

#include <QPlainTextEdit>
#include <QSharedPointer>
#include <QString>
#include <QWidget>

#include "axserve/common/logging/text_edit_message_appender.h"
#include "axserve/internal/server/active_server.h"

class RunningServerWidget : public QWidget {
  Q_OBJECT

public:
  enum FailedReason {
    NONE,
    CONTROL,
    SERVER,
  };

private:
  QPlainTextEdit *m_edit;
  QSharedPointer<PlainTextEditMessageAppender> m_appender;

  ActiveServer *m_server;

  bool m_isReady;
  FailedReason m_failedReason;

public:
  RunningServerWidget(
      const QString &control, const QString &address, QWidget *parent = nullptr,
      Qt::WindowFlags f = Qt::WindowFlags()
  );

  bool isReady() const;
  FailedReason failedReason() const;
};

#endif // RUNNING_SERVER_WIDGET_H
