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
// SPDX-FileCopyrightText: 2025 Yunseong Hwang
//
// SPDX-License-Identifier: Apache-2.0

#include "outbound_channel.h"

#include <QMutexLocker>

#include "inbound_item.h"
#include "outbound_item.h"

OutboundChannel::OutboundChannel() {}

OutboundChannel::OutboundChannel(const QWeakPointer<OutboundReceiver> &receiver)
    : m_receiver(receiver) {}

void OutboundChannel::subscribe(
    const QWeakPointer<OutboundReceiver> &receiver
) {
  m_receiver = receiver;
}

void OutboundChannel::send(const QSharedPointer<OutboundItem> &outbound) {
  {
    QMutexLocker<QMutex> locker(&m_runningMutex);
    m_running[outbound->uuid()] = outbound;
  }
  {
    QMutexLocker<QMutex> locker(&m_pendingMutex);
    m_pending.enqueue(outbound);
  }
  QSharedPointer<OutboundReceiver> receiver = m_receiver;
  if (receiver) {
    receiver->notifyOutbound();
  }
}

QQueue<QSharedPointer<OutboundItem>> OutboundChannel::receive() {
  QMutexLocker<QMutex> locker(&m_pendingMutex);
  QQueue<QSharedPointer<OutboundItem>> outbounds = std::move(m_pending);
  m_pending.clear();
  return std::move(outbounds);
}

bool OutboundChannel::finish(const QSharedPointer<OutboundItem> &outbound) {
  QMutexLocker<QMutex> locker(&m_runningMutex);
  return m_running.remove(outbound->uuid());
}

void OutboundChannel::send(const QSharedPointer<InboundItem> &inbound) {
  ContextType context = inbound->contextType();
  if (context != ContextType::EVENT) {
    return;
  }
  QUuid uuid = inbound->contextId();
  QMutexLocker<QMutex> locker(&m_runningMutex);
  if (!m_running.contains(uuid)) {
    return;
  }
  QSharedPointer<OutboundItem> outbound = m_running.value(uuid);
  outbound->send(inbound);
}
