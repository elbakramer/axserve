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

#ifndef HANDLE_EVENT_COUNTER_H
#define HANDLE_EVENT_COUNTER_H

#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QWaitCondition>
#include <QWeakPointer>

class HandleEventCounter {
private:
  int m_id;
  int m_count;
  bool m_started;

  QList<QWeakPointer<QWaitCondition>> m_notificationList;

  QMutex m_countMutex;
  QMutex m_notificationListMutex;

public:
  explicit HandleEventCounter(int id, int count = 0);

  int id();
  int count();

  void setCount(int count);

  void registerNotification(const QSharedPointer<QWaitCondition> &cond);
  void unregisterNotification(const QSharedPointer<QWaitCondition> &cond);

  void handleOne();
  void handleAll();

  bool isDone();
};

#endif
