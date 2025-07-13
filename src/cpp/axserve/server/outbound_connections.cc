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

#include "outbound_connections.h"

#include <QMutexLocker>

#include "axserve/common/logging/protobuf_message_logging.h"

int OutboundConnections::connect(const QString &peer) {
  QMutexLocker<QMutex> locker(&m_mutex);
  return m_connections[peer]++;
}

int OutboundConnections::disconnect(const QString &peer, bool entire) {
  QMutexLocker<QMutex> locker(&m_mutex);
  if (!m_connections.contains(peer)) {
    return 0;
  }
  if (entire) {
    m_connections.remove(peer);
    return 0;
  }
  int count = m_connections[peer]--;
  if (count <= 0) {
    m_connections.remove(peer);
    return 0;
  }
  return count;
}

bool OutboundConnections::contains(const QString &peer) {
  QMutexLocker<QMutex> locker(&m_mutex);
  return m_connections.contains(peer);
}

QStringList OutboundConnections::connections() {
  QMutexLocker<QMutex> locker(&m_mutex);
  return m_connections.keys();
}
