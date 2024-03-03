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

#include "handle_event_reactor.h"
#include "handle_event_reactors_manager.h"

#include <QMutexLocker>

#include <QtDebug>
#include <QtLogging>

using grpc::Status;
using grpc::StatusCode;

HandleEventReactor::HandleEventReactor(
    const QSharedPointer<HandleEventReactorsManager> &manager,
    const QString &peer
)
    : m_manager(manager),
      m_peer(peer),
      m_done(false),
      m_writing(false) {
  StartRead(&m_response);
}

QString HandleEventReactor::peer() { return m_peer; }

bool HandleEventReactor::startHandle(
    const QSharedPointer<HandleEventRequest> &request,
    const QSharedPointer<HandleEventCounter> &counter
) {
  QMutexLocker<QMutex> lock(&m_mutex);
  if (m_done) {
    return false;
  }
  m_requests.append(request);
  m_counters[counter->id()] = counter;
  if (!m_writing) {
    NextWrite();
  }
  return true;
}

void HandleEventReactor::NextWrite() {
  if (m_requests.size() > 0) {
    auto &weak_request = m_requests.first();
    auto strong_request = weak_request.toStrongRef();
    while (!strong_request && m_requests.size() > 0) {
      m_requests.removeFirst();
      weak_request = m_requests.first();
      strong_request = weak_request.toStrongRef();
    }
    if (strong_request) {
      m_writing = true;
      StartWrite(strong_request.get());
      return;
    }
  }
  m_writing = false;
}

void HandleEventReactor::OnDone() {
  {
    QMutexLocker<QMutex> lock(&m_mutex);
    m_done = true;
    for (auto &weak_counter : m_counters) {
      auto strong_counter = weak_counter.toStrongRef();
      if (strong_counter) {
        strong_counter->handleOne();
      }
    }
    m_counters.clear();
    m_requests.clear();
  }
  auto manager = m_manager.toStrongRef();
  if (manager) {
    manager->deleteReactor(this);
  }
}

void HandleEventReactor::OnReadDone(bool ok) {
  if (ok) {
    if (m_response.is_ping()) {
      HandleEventRequest pong_request;
      pong_request.set_is_pong(true);
      StartWrite(&pong_request);
    } else {
      QMutexLocker<QMutex> lock(&m_mutex);
      int id = m_response.id();
      if (m_counters.contains(id)) {
        auto &weak_counter = m_counters[id];
        auto strong_counter = weak_counter.toStrongRef();
        if (strong_counter) {
          strong_counter->handleOne();
        }
        m_counters.remove(id);
      }
    }
    StartRead(&m_response);
  } else {
    Status status(
        StatusCode::OK, "Client stopped sending further HandleEventReponse"
    );
    Finish(status);
  }
}

void HandleEventReactor::OnWriteDone(bool ok) {
  if (ok) {
    QMutexLocker<QMutex> lock(&m_mutex);
    if (m_requests.size() > 0) {
      m_requests.removeFirst();
    }
    NextWrite();
  } else {
    Status status(StatusCode::UNKNOWN, "Failed to send HandleEventRequest");
    Finish(status);
  }
}
