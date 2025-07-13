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

#include "outbound_reactor.h"

#include <QMutexLocker>

#include "executor.h"
#include "outbound_item.h"

OutboundReactor::OutboundReactor(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context
)
    : m_executor(executor) {
  m_uuid = QUuid::createUuid();
  m_peer = QString::fromStdString(context->peer());
  m_writing = false;
  m_pongs = 0;
  m_pong.set_is_pong(true);
}

QSharedPointer<OutboundReactor> OutboundReactor::create(
    const QSharedPointer<Executor> &executor, CallbackServerContext *context
) {
  QSharedPointer<OutboundReactor> reactor =
      QSharedPointer<OutboundReactor>::create(executor, context);
  reactor->initialize();
  return reactor;
}

void OutboundReactor::initialize() {
  QSharedPointer<Executor> executor = m_executor;
  QSharedPointer<OutboundReactor> reactor = sharedFromThis();
  if (executor && reactor) {
    executor->establish(reactor);
  }
  StartRead(&m_response);
}

const QUuid &OutboundReactor::uuid() const { return m_uuid; }
const QString &OutboundReactor::peer() const { return m_peer; }

void OutboundReactor::send(const QSharedPointer<OutboundItem> &item) {
  {
    QMutexLocker<QMutex> pendingLock(&m_pendingMutex);
    m_pending.enqueue(item);
  }
  NextWrite();
}

void OutboundReactor::NextWrite() {
  {
    QMutexLocker<QMutex> writingLock(&m_writingMutex);
    if (m_writing) {
      return;
    }
  }
  {
    QMutexLocker<QMutex> writingLock(&m_pongsMutex);
    if (m_pongs > 0) {
      StartWrite(&m_pong);
      m_pongs--;
      return;
    }
  }
  QSharedPointer<OutboundItem> item;
  {
    QMutexLocker<QMutex> pendingLock(&m_pendingMutex);
    if (m_pending.empty()) {
      return;
    } else {
      QMutexLocker<QMutex> writingLock(&m_writingMutex);
      if (m_writing) {
        return;
      } else {
        m_writing = true;
      }
    }
    item = m_pending.dequeue();
  }
  {
    QMutexLocker<QMutex> runningLock(&m_runningMutex);
    m_running[item->uuid()] = item;
  }
  StartWrite(&item->request());
}

void OutboundReactor::OnDone() {
  QSharedPointer<OutboundReactor> reactor = sharedFromThis();
  {
    QMutexLocker<QMutex> runningLock(&m_runningMutex);
    for (const QSharedPointer<OutboundItem> &item : m_running.values()) {
      if (item && reactor) {
        item->notifyHandledBy(reactor);
      }
    }
    m_running.clear();
  }
  {
    QMutexLocker<QMutex> pendingLock(&m_pendingMutex);
    for (const QSharedPointer<OutboundItem> &item : m_pending) {
      if (item && reactor) {
        item->notifyHandledBy(reactor);
      }
    }
    m_pending.clear();
  }
  {
    QSharedPointer<Executor> executor = m_executor;
    if (executor && reactor) {
      executor->dissolve(reactor);
    }
  }
}

void OutboundReactor::OnReadDone(bool ok) {
  if (!ok) {
    std::string msg = "Client stopped sending further response";
    Status status(StatusCode::OK, msg);
    Finish(status);
  } else if (m_response.is_ping()) {
    {
      QMutexLocker<QMutex> writingLock(&m_pongsMutex);
      m_pongs++;
    }
    StartRead(&m_response);
  } else {
    QSharedPointer<OutboundItem> item;
    {
      QMutexLocker<QMutex> runningLock(&m_runningMutex);
      QUuid uuid = QUuid::fromString(m_response.id());
      item = m_running.take(uuid);
    }
    QSharedPointer<OutboundReactor> reactor = sharedFromThis();
    if (item && reactor) {
      item->notifyHandledBy(reactor);
    }
    StartRead(&m_response);
  }
}

void OutboundReactor::OnWriteDone(bool ok) {
  {
    QMutexLocker<QMutex> writingLock(&m_writingMutex);
    m_writing = false;
  }
  if (!ok) {
    std::string msg = "Failed to send request";
    Status status(StatusCode::UNKNOWN, msg);
    Finish(status);
  } else {
    NextWrite();
  }
}
