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

#ifndef HANDLE_EVENT_REACTOR_H
#define HANDLE_EVENT_REACTOR_H

#include <QHash>
#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QWeakPointer>

#include "active.grpc.pb.h"

#include "handle_event_counter.h"

using grpc::ServerBidiReactor;

class HandleEventReactorsManager;

class HandleEventReactor
    : public ServerBidiReactor<HandleEventResponse, HandleEventRequest> {
private:
  QWeakPointer<HandleEventReactorsManager> m_manager;
  QString m_peer;

  bool m_done;
  bool m_writing;

  HandleEventResponse m_response;

  QList<QWeakPointer<HandleEventRequest>> m_requests;
  QHash<int, QWeakPointer<HandleEventCounter>> m_counters;

  QMutex m_mutex;

public:
  HandleEventReactor(
      const QSharedPointer<HandleEventReactorsManager> &manager,
      const QString &peer
  );

public:
  QString peer();

  bool startHandle(
      const QSharedPointer<HandleEventRequest> &request,
      const QSharedPointer<HandleEventCounter> &counter
  );

private:
  void NextWrite();

public:
  void OnDone() override;
  void OnReadDone(bool ok) override;
  void OnWriteDone(bool ok) override;
};

#endif
