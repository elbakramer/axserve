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

#include "outbound_reactor_set.h"

#include <QMutexLocker>

#include "outbound_connections.h"
#include "outbound_item.h"
#include "outbound_reactor.h"

#include "axserve/common/logging/protobuf_message_logging.h"

QList<QSharedPointer<OutboundReactor>>
OutboundReactorSet::reactorsFor(const QSharedPointer<OutboundItem> &item) {
  QMutexLocker<QMutex> locker(&m_mutex);
  QList<QSharedPointer<OutboundReactor>> reactors;
  QSharedPointer<OutboundConnections> connections = item->connections();
  if (!connections) {
    return std::move(reactors);
  }
  QList<QString> peers = connections->connections();
  if (peers.empty()) {
    return std::move(reactors);
  }
  for (const QString &peer : peers) {
    reactors.append(m_reactors.value(peer).values());
  }
  return std::move(reactors);
}

void OutboundReactorSet::establish(
    const QSharedPointer<OutboundReactor> &reactor
) {
  QMutexLocker<QMutex> locker(&m_mutex);
  m_reactors[reactor->peer()][reactor->uuid()] = reactor;
}

bool OutboundReactorSet::dissolve(
    const QSharedPointer<OutboundReactor> &reactor
) {
  QMutexLocker<QMutex> locker(&m_mutex);
  bool removed = m_reactors.value(reactor->peer()).remove(reactor->uuid());
  if (m_reactors.value(reactor->peer()).empty()) {
    m_reactors.remove(reactor->peer());
  }
  return removed;
}

void OutboundReactorSet::send(const QSharedPointer<OutboundItem> &item) {
  item->sendTo(reactorsFor(item));
}
