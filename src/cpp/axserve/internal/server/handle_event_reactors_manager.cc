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

#include "handle_event_reactors_manager.h"

#include <QtDebug>
#include <QtLogging>

#include <QMutexLocker>

HandleEventReactor *
HandleEventReactorsManager::createReactor(const QString &peer) {
  QMutexLocker<QMutex> lock(&m_reactorsMutex);
  HandleEventReactor *reactor = new HandleEventReactor(sharedFromThis(), peer);
  m_reactors[peer].append(reactor);
  return reactor;
}

void HandleEventReactorsManager::deleteReactor(HandleEventReactor *reactor) {
  QMutexLocker<QMutex> lock(&m_reactorsMutex);
  QString peer = reactor->peer();
  m_reactors[peer].removeAll(reactor);
  delete reactor;
  if (m_reactors[peer].size() == 0) {
    m_reactors.remove(peer);
    {
      QMutexLocker<QMutex> lock(&m_connectionsMutex);
      m_connections.remove(peer);
    }
  }
}

void HandleEventReactorsManager::connectEvent(const QString &peer, int index) {
  QMutexLocker<QMutex> lock(&m_connectionsMutex);
  m_connections[peer][index] += 1;
}

void HandleEventReactorsManager::disconnectEvent(
    const QString &peer, int index
) {
  QMutexLocker<QMutex> lock(&m_connectionsMutex);
  m_connections[peer][index] -= 1;
  if (m_connections[peer][index] < 0) {
    m_connections[peer][index] = 0;
  }
  if (m_connections[peer][index] == 0) {
    m_connections[peer].remove(index);
    if (m_connections[peer].size() == 0) {
      m_connections.remove(peer);
    }
  }
}

void HandleEventReactorsManager::startHandle(
    int index, const QSharedPointer<HandleEventRequest> &request,
    const QSharedPointer<HandleEventCounter> &counter
) {
  QMutexLocker<QMutex> lock1(&m_reactorsMutex);
  QMutexLocker<QMutex> lock2(&m_connectionsMutex);
  QList<HandleEventReactor *> reactors;
  for (auto [peer, counts] : m_connections.asKeyValueRange()) {
    if (counts[index] > 0) {
      reactors << m_reactors[peer];
    }
  }
  if (reactors.size() == 0) {
    return;
  }
  counter->setCount(reactors.size());
  for (auto reactor : reactors) {
    reactor->startHandle(request, counter);
  }
}
