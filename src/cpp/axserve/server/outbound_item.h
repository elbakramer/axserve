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

#ifndef OUTBOUND_ITEM_H
#define OUTBOUND_ITEM_H

#include <QDateTime>
#include <QDeadlineTimer>
#include <QEnableSharedFromThis>
#include <QFuture>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QPromise>
#include <QSharedPointer>
#include <QUuid>
#include <QVariantList>
#include <QWaitCondition>

#include "active.grpc.pb.h"

using namespace axserve;

#include "inbound_channel.h"

class InboundItem;
class OutboundConnections;
class OutboundReactor;

class OutboundItem : public InboundReceiver,
                     public QEnableSharedFromThis<OutboundItem> {
private:
  QUuid m_uuid;
  HandleEventRequest m_request;
  QSharedPointer<OutboundConnections> m_connections;
  InboundChannel m_inbounds;
  QHash<QUuid, QSharedPointer<OutboundReactor>> m_reactors;
  QMutex m_reactorsMutex;
  QWaitCondition m_condition;
  QMutex m_conditionMutex;
  QPromise<void> m_promise;
  QFuture<void> m_future;

private:
  OutboundItem(
      const QDateTime &timestamp, const QUuid &instance, int index,
      const QVariantList &args,
      const QSharedPointer<OutboundConnections> &connections
  );

public:
  static QSharedPointer<OutboundItem> create(
      const QDateTime &timestamp, const QUuid &instance, int index,
      const QVariantList &args,
      const QSharedPointer<OutboundConnections> &connections
  );

private:
  friend class QSharedPointer<OutboundItem>;
  void initialize();

public:
  const QUuid &uuid();
  const HandleEventRequest &request();
  const QSharedPointer<OutboundConnections> &connections();

  bool started() const;
  bool running() const;
  bool finished() const;
  bool canceled() const;

  void start();
  void finish();
  void cancel();

  void sendTo(const QList<QSharedPointer<OutboundReactor>> &reactors);
  void notifyHandledBy(const QSharedPointer<OutboundReactor> &reactor);

  bool done() const;
  void notifyInbound() override;

  void send(const QSharedPointer<InboundItem> &item);
  QQueue<QSharedPointer<InboundItem>> receive();

  bool wait(QDeadlineTimer deadline = QDeadlineTimer(QDeadlineTimer::Forever));
};

#endif // OUTBOUND_ITEM_H
