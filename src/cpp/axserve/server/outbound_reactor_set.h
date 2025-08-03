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
 * SPDX-FileCopyrightText: 2025 Yunseong Hwang
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OUTBOUND_REACTOR_SET_H
#define OUTBOUND_REACTOR_SET_H

#include <QHash>
#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QUuid>

class OutboundItem;
class OutboundReactor;

class OutboundReactorSet {
private:
  QHash<QString, QHash<QUuid, QSharedPointer<OutboundReactor>>> m_reactors;
  QMutex m_mutex;

private:
  QList<QSharedPointer<OutboundReactor>>
  reactorsFor(const QSharedPointer<OutboundItem> &item);

public:
  void establish(const QSharedPointer<OutboundReactor> &reactor);
  bool dissolve(const QSharedPointer<OutboundReactor> &reactor);
  void send(const QSharedPointer<OutboundItem> &item);
};

#endif // OUTBOUND_REACTOR_SET_H
