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

#ifndef ACTIVE_SERVICE_REQUEST_QUEUE_H
#define ACTIVE_SERVICE_REQUEST_QUEUE_H

#include <QList>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QWeakPointer>

#include "model/active_service_request.h"

class ActiveServiceRequestQueue {
private:
  QQueue<ActiveServiceRequest *> m_queue;
  QList<QWeakPointer<QWaitCondition>> m_notificationList;

  QMutex m_queueMutex;
  QMutex m_notificationListMutex;

public:
  void registerNotification(const QSharedPointer<QWaitCondition> &cond);
  void unregisterNotification(const QSharedPointer<QWaitCondition> &cond);

  void pushRequest(ActiveServiceRequest *request);
  ActiveServiceRequest *popRequest();
};

#endif
