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

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <QObject>
#include <QSharedPointer>

#include "inbound_channel.h"
#include "inbound_item.h"
#include "outbound_channel.h"
#include "outbound_item.h"

using grpc::Status;
using grpc::StatusCode;

class ControlSet;
class OutboundReactorSet;

class Executor : public QObject,
                 public InboundReceiver,
                 public OutboundReceiver,
                 public QEnableSharedFromThis<Executor> {
  Q_OBJECT

private:
  QSharedPointer<ControlSet> m_controls;
  QSharedPointer<InboundChannel> m_inbounds;
  QSharedPointer<OutboundChannel> m_outbounds;
  QSharedPointer<OutboundReactorSet> m_reactors;

private:
  Executor();

public:
  static QSharedPointer<Executor> create();

private:
  friend class QSharedPointer<Executor>;
  void initialize();

public:
  bool addControl(const QString &classId);

signals:
  void inbound();
  void outbound();

public slots:
  void handleInbound();
  void handleOutbound();

public:
  void notifyInbound() override;
  void notifyOutbound() override;

  void schedule(const QSharedPointer<InboundItem> &item);
  void schedule(const QSharedPointer<OutboundItem> &item);

  bool execute(const QSharedPointer<OutboundItem> &item);
  bool execute(const QSharedPointer<InboundItem> &item);
  bool execute(const QSharedPointer<CreateInboundItem> &item);
  bool execute(const QSharedPointer<ReferInboundItem> &item);
  bool execute(const QSharedPointer<ReleaseInboundItem> &item);
  bool execute(const QSharedPointer<DestroyInboundItem> &item);
  bool execute(const QSharedPointer<ListInboundItem> &item);
  bool execute(const QSharedPointer<DescribeInboundItem> &item);
  bool execute(const QSharedPointer<GetPropertyInboundItem> &item);
  bool execute(const QSharedPointer<SetPropertyInboundItem> &item);
  bool execute(const QSharedPointer<InvokeMethodInboundItem> &item);
  bool execute(const QSharedPointer<ConnectEventInboundItem> &item);
  bool execute(const QSharedPointer<DisconnectEventInboundItem> &item);

  void establish(const QSharedPointer<OutboundReactor> &reactor);
  bool dissolve(const QSharedPointer<OutboundReactor> &reactor);
};

#endif // EXECUTOR_H
