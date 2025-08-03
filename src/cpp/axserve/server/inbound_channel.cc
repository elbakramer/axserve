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

#include "inbound_channel.h"

#include <QMutexLocker>

#include "inbound_item.h"

InboundChannel::InboundChannel() {}

InboundChannel::InboundChannel(const QWeakPointer<InboundReceiver> &receiver)
    : m_receiver(receiver) {}

void InboundChannel::subscribe(const QWeakPointer<InboundReceiver> &receiver) {
  m_receiver = receiver;
}

void InboundChannel::send(const QSharedPointer<InboundItem> &inbound) {
  QMutexLocker<QMutex> locker(&m_mutex);
  m_queue.enqueue(inbound);
  QSharedPointer<InboundReceiver> receiver = m_receiver;
  if (receiver) {
    receiver->notifyInbound();
  }
}

QQueue<QSharedPointer<InboundItem>> InboundChannel::receive() {
  QMutexLocker<QMutex> locker(&m_mutex);
  QQueue<QSharedPointer<InboundItem>> inbounds = std::move(m_queue);
  m_queue.clear();
  return std::move(inbounds);
}
