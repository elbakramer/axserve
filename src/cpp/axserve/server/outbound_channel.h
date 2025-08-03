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

#ifndef OUTBOUND_CHANNEL_H
#define OUTBOUND_CHANNEL_H

#include <QHash>
#include <QMutex>
#include <QQueue>
#include <QSharedPointer>
#include <QUuid>
#include <QWeakPointer>

class InboundItem;
class OutboundItem;

class OutboundReceiver {
public:
  virtual void notifyOutbound() = 0;
};

class OutboundChannel {
private:
  QWeakPointer<OutboundReceiver> m_receiver;
  QQueue<QSharedPointer<OutboundItem>> m_pending;
  QHash<QUuid, QSharedPointer<OutboundItem>> m_running;
  QMutex m_pendingMutex;
  QMutex m_runningMutex;

public:
  OutboundChannel();
  OutboundChannel(const QWeakPointer<OutboundReceiver> &receiver);

  void subscribe(const QWeakPointer<OutboundReceiver> &receiver);
  void send(const QSharedPointer<OutboundItem> &outbound);
  QQueue<QSharedPointer<OutboundItem>> receive();

  bool finish(const QSharedPointer<OutboundItem> &outbound);
  void send(const QSharedPointer<InboundItem> &inbound);
};

#endif // OUTBOUND_CHANNEL_H
