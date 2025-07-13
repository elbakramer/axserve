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

#ifndef OUTBOUND_CONNECTIONS_H
#define OUTBOUND_CONNECTIONS_H

#include <QAtomicInteger>
#include <QHash>
#include <QMutex>
#include <QString>
#include <QStringList>

class OutboundConnections {
private:
  QHash<QString, QAtomicInteger<int>> m_connections;
  QMutex m_mutex;

public:
  int connect(const QString &peer);
  int disconnect(const QString &peer, bool entire = false);
  bool contains(const QString &peer);
  QStringList connections();
};

#endif // OUTBOUND_CONNECTIONS_H
