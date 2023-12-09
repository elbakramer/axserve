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

#ifndef MESSAGE_HANDLERS_MANAGER_H
#define MESSAGE_HANDLERS_MANAGER_H

#include <functional>

#include <QtLogging>

#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QWeakPointer>

#include "message_handler.h"

class MessageHandlersManager : public AbstractMessageHandler {
private:
  static MessageHandlersManager *m_instance;
  static QMutex m_instance_mutex;

private:
  QMutex m_mutex;
  QList<QtMessageHandler> m_handler_fns;
  QList<QWeakPointer<AbstractMessageHandler>> m_handlers;

private:
  MessageHandlersManager();

public:
  virtual ~MessageHandlersManager();

public:
  static MessageHandlersManager *instance();
  static void messageHandler(
      QtMsgType type, const QMessageLogContext &context, const QString &str
  );

  void registerHandler(QtMessageHandler handler);
  void unregisterHandler(QtMessageHandler handler);

  void registerHandler(const QSharedPointer<AbstractMessageHandler> &handler);
  void unregisterHandler(const QSharedPointer<AbstractMessageHandler> &handler);

  void operator()(
      QtMsgType type, const QMessageLogContext &context, const QString &str
  ) override;
};

#endif // MESSAGE_HANDLERS_MANAGER_H
