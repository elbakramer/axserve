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
// SPDX-FileCopyrightText: 2025 Yunseong Hwang
//
// SPDX-License-Identifier: Apache-2.0

#include "outbound_item.h"

#include <QMutexLocker>

#include "inbound_channel.h"
#include "inbound_item.h"
#include "outbound_connections.h"
#include "outbound_reactor.h"

#include "util/variant_conversion.h"

#include "axserve/common/logging/protobuf_message_logging.h"

OutboundItem::OutboundItem(
    const QDateTime &timestamp, const QUuid &instance, int index,
    const QVariantList &args,
    const QSharedPointer<OutboundConnections> &connections
) {
  m_uuid = QUuid::createUuid();
  m_request.set_timestamp(timestamp.toMSecsSinceEpoch());
  m_request.set_id(m_uuid.toString().toStdString());
  m_request.set_instance(instance.toString().toStdString());
  m_request.set_index(index);
  for (const QVariant &qt_arg : args) {
    Variant &proto_arg = *m_request.add_arguments();
    QVariantToProtoVariant(qt_arg, proto_arg);
  }
  m_connections = connections;
  m_future = m_promise.future();
}

QSharedPointer<OutboundItem> OutboundItem::create(
    const QDateTime &timestamp, const QUuid &instance, int index,
    const QVariantList &args,
    const QSharedPointer<OutboundConnections> &connections
) {
  QSharedPointer<OutboundItem> item = QSharedPointer<OutboundItem>::create(
      timestamp, instance, index, args, connections
  );
  item->initialize();
  return item;
}

void OutboundItem::initialize() {
  QSharedPointer<OutboundItem> item = sharedFromThis();
  if (item) {
    m_inbounds.subscribe(item);
  }
}

const QUuid &OutboundItem::uuid() { return m_uuid; }
const HandleEventRequest &OutboundItem::request() { return m_request; }
const QSharedPointer<OutboundConnections> &OutboundItem::connections() {
  return m_connections;
}

bool OutboundItem::started() const { return m_future.isStarted(); }
bool OutboundItem::running() const { return m_future.isRunning(); }
bool OutboundItem::finished() const { return m_future.isFinished(); }
bool OutboundItem::canceled() const { return m_promise.isCanceled(); }

void OutboundItem::start() { m_promise.start(); }
void OutboundItem::finish() {
  m_promise.finish();
  m_condition.wakeOne();
}
void OutboundItem::cancel() {
  m_future.cancel();
  m_condition.wakeOne();
}

void OutboundItem::sendTo(
    const QList<QSharedPointer<OutboundReactor>> &reactors
) {
  if (started()) {
    return;
  }
  if (reactors.empty()) {
    finish();
    return;
  }
  start();
  QMutexLocker<QMutex> locker(&m_reactorsMutex);
  for (const QSharedPointer<OutboundReactor> &reactor : reactors) {
    m_reactors[reactor->uuid()] = reactor;
  }
  for (const QSharedPointer<OutboundReactor> &reactor : reactors) {
    reactor->send(sharedFromThis());
  }
}

void OutboundItem::notifyHandledBy(
    const QSharedPointer<OutboundReactor> &reactor
) {
  if (!started()) {
    return;
  }
  QMutexLocker<QMutex> locker(&m_reactorsMutex);
  m_reactors.remove(reactor->uuid());
  if (m_reactors.empty()) {
    finish();
  }
}

bool OutboundItem::done() const { return finished() || canceled(); }

void OutboundItem::notifyInbound() { m_condition.wakeOne(); }

void OutboundItem::send(const QSharedPointer<InboundItem> &item) {
  m_inbounds.send(item);
}

QQueue<QSharedPointer<InboundItem>> OutboundItem::receive() {
  return m_inbounds.receive();
}

bool OutboundItem::wait(QDeadlineTimer deadline) {
  QMutexLocker<QMutex> locker(&m_conditionMutex);
  return m_condition.wait(&m_conditionMutex, deadline);
}
