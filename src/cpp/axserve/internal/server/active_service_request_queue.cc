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

#include "active_service_request_queue.h"

#include <QMutexLocker>

void ActiveServiceRequestQueue::registerNotification(
    const QSharedPointer<QWaitCondition> &cond
) {
  QMutexLocker<QMutex> lock(&m_notificationListMutex);
  m_notificationList.append(cond);
}

void ActiveServiceRequestQueue::unregisterNotification(
    const QSharedPointer<QWaitCondition> &cond
) {
  QMutexLocker<QMutex> lock(&m_notificationListMutex);
  m_notificationList.removeOne(cond);
}

void ActiveServiceRequestQueue::pushRequest(ActiveServiceRequest *request) {
  {
    QMutexLocker<QMutex> lock(&m_queueMutex);
    m_queue.append(request);
  }
  {
    QMutexLocker<QMutex> lock(&m_notificationListMutex);
    for (const auto &wc : m_notificationList) {
      auto sc = wc.toStrongRef();
      if (sc) {
        sc->wakeOne();
      } else {
        m_notificationList.removeAll(wc);
      }
    }
  }
}

ActiveServiceRequest *ActiveServiceRequestQueue::popRequest() {
  QMutexLocker<QMutex> lock(&m_queueMutex);
  ActiveServiceRequest *request = nullptr;
  if (!m_queue.empty()) {
    request = m_queue.first();
    m_queue.removeFirst();
  }
  return request;
}
