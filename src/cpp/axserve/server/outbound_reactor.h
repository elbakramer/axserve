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

#ifndef OUTBOUND_REACTOR_H
#define OUTBOUND_REACTOR_H

#include <QEnableSharedFromThis>
#include <QHash>
#include <QMutex>
#include <QQueue>
#include <QSharedPointer>
#include <QString>
#include <QUuid>
#include <QWeakPointer>

#include "active.grpc.pb.h"

using grpc::CallbackServerContext;
using grpc::ServerBidiReactor;

using namespace axserve;

class Executor;
class OutboundItem;

class OutboundReactor
    : public ServerBidiReactor<HandleEventResponse, HandleEventRequest>,
      public QEnableSharedFromThis<OutboundReactor> {
private:
  QUuid m_uuid;
  QString m_peer;
  QWeakPointer<Executor> m_executor;
  QQueue<QSharedPointer<OutboundItem>> m_pending;
  QHash<QUuid, QSharedPointer<OutboundItem>> m_running;
  bool m_writing;
  int m_pongs;
  QMutex m_pendingMutex;
  QMutex m_runningMutex;
  QMutex m_writingMutex;
  QMutex m_pongsMutex;
  HandleEventResponse m_response;
  HandleEventRequest m_pong;

private:
  OutboundReactor(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context
  );

public:
  static QSharedPointer<OutboundReactor> create(
      const QSharedPointer<Executor> &executor, CallbackServerContext *context
  );

private:
  friend class QSharedPointer<OutboundReactor>;
  void initialize();

public:
  const QUuid &uuid() const;
  const QString &peer() const;

  void send(const QSharedPointer<OutboundItem> &item);

private:
  void NextWrite();

public:
  void OnDone() override;
  void OnReadDone(bool ok) override;
  void OnWriteDone(bool ok) override;
};

#endif // OUTBOUND_REACTOR_H
