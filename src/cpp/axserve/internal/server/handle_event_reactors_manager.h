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

#ifndef HANDLE_EVENT_REACTORS_MANAGER_H
#define HANDLE_EVENT_REACTORS_MANAGER_H

#include <QEnableSharedFromThis>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QString>

#include "active.pb.h"

#include "handle_event_counter.h"
#include "handle_event_reactor.h"

class HandleEventReactorsManager
    : public QEnableSharedFromThis<HandleEventReactorsManager> {
private:
  QHash<QString, QList<HandleEventReactor *>> m_reactors;
  QMutex m_reactorsMutex;

  QHash<QString, QHash<int, int>> m_connections;
  QMutex m_connectionsMutex;

public:
  HandleEventReactor *createReactor(const QString &peer);
  void deleteReactor(HandleEventReactor *reactor);

  void connectEvent(const QString &peer, int index);
  void disconnectEvent(const QString &peer, int index);

  void startHandle(
      int index, const QSharedPointer<HandleEventRequest> &request,
      const QSharedPointer<HandleEventCounter> &counter
  );
};

#endif
