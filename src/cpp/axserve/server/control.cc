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

#include "control.h"

#include <exception>
#include <sstream>

#include "executor.h"
#include "inbound_item.h"
#include "outbound_connections.h"
#include "outbound_item.h"

#include "util/generic_invoke_method.h"
#include "util/variant_conversion.h"

#include "axserve/common/logging/protobuf_message_logging.h"

#include <QtLogging>

Control::Description::Description() {}
Control::Description::Description(
    QVector<QMetaProperty> &&properties, QVector<QMetaMethod> &&methods,
    QVector<QMetaMethod> &&events
)
    : m_properties(properties),
      m_methods(methods),
      m_events(events) {}

const QVector<QMetaProperty> &Control::Description::properties() const {
  return m_properties;
}
const QVector<QMetaMethod> &Control::Description::methods() const {
  return m_methods;
}
const QVector<QMetaMethod> &Control::Description::events() const {
  return m_events;
}

Control::Control(const QWeakPointer<Executor> &executor, const QString &c) {
  m_executor = executor;
  m_control.reset(new QAxWidget());
  m_description.reset(new Description());
  bool successful = m_control->setControl(c);
  if (!successful) {
    m_control.reset();
    return;
  }
  m_uuid = QUuid::createUuid();
  m_clsid = m_control->control();
  QVector<QMetaProperty> m_properties;
  QVector<QMetaMethod> m_methods;
  QVector<QMetaMethod> m_events;
  const QMetaObject *meta = m_control->metaObject();
  int propOffset = meta->propertyOffset();
  int propCount = meta->propertyCount();
  for (int i = propOffset; i < propCount; i++) {
    QMetaProperty prop = meta->property(i);
    if (!prop.isValid()) {
      continue;
    }
    m_properties.append(prop);
  }
  int methodOffest = meta->methodOffset();
  int methodCount = meta->methodCount();
  for (int i = methodOffest; i < methodCount; i++) {
    QMetaMethod method = meta->method(i);
    if (!method.isValid()) {
      continue;
    }
    QMetaMethod::MethodType methodType = method.methodType();
    if (methodType == QMetaMethod::MethodType::Slot) {
      m_methods.append(method);
    } else if (methodType == QMetaMethod::MethodType::Signal) {
      QString name = method.methodSignature();
      m_eventIndices[name] = m_events.size();
      m_events.append(method);
    }
  }
  Description *description = new Description(
      std::move(m_properties), std::move(m_methods), std::move(m_events)
  );
  m_description.reset(description);
  connect(m_control.get(), &QAxWidget::signal, this, &Control::slot);
}

bool Control::initialized() {
  return !m_control.isNull() && !m_control->isNull();
}

const QUuid &Control::instance() { return m_uuid; }

const QString &Control::control() { return m_clsid; }

const Control::Description &Control::describe() { return *m_description; }

QVariant Control::getProperty(int index) {
  auto const &properties = m_description->properties();
  bool contains = index >= 0 && index < properties.size();
  if (!contains) {
    std::stringstream ss;
    ss << "Given index " << index << " is out of range [" << 0 << ", "
       << properties.size() << ")";
    throw std::out_of_range(ss.str());
  }
  return properties.at(index).read(m_control.get());
}

bool Control::setProperty(int index, const QVariant &value) {
  auto const &properties = m_description->properties();
  bool contains = index >= 0 && index < properties.size();
  if (!contains) {
    std::stringstream ss;
    ss << "Given index " << index << " is out of range [" << 0 << ", "
       << properties.size() << ")";
    throw std::out_of_range(ss.str());
  }
  return properties.at(index).write(m_control.get(), value);
}

bool Control::setProperty(int index, QVariant &&value) {
  auto const &properties = m_description->properties();
  bool contains = index >= 0 && index < properties.size();
  if (!contains) {
    std::stringstream ss;
    ss << "Given index " << index << " is out of range [" << 0 << ", "
       << properties.size() << ")";
    throw std::out_of_range(ss.str());
  }
  return properties.at(index).write(m_control.get(), value);
}

QVariant Control::invokeMethod(
    int index, const QVariantList &args, Qt::ConnectionType connection_type
) {
  auto const &methods = m_description->methods();
  bool contains = index >= 0 && index < methods.size();
  if (!contains) {
    std::stringstream ss;
    ss << "Given index " << index << " is out of range [" << 0 << ", "
       << methods.size() << ")";
    throw std::out_of_range(ss.str());
  }
  return GenericInvokeMethod(
      m_control.get(), methods.at(index), args, connection_type
  );
}

bool Control::connectEvent(int index, const QString &peer) {
  auto const &events = m_description->events();
  bool contains = index >= 0 && index < events.size();
  if (!contains) {
    std::stringstream ss;
    ss << "Given index " << index << " is out of range [" << 0 << ", "
       << events.size() << ")";
    throw std::out_of_range(ss.str());
  }
  if (!m_eventConnections.contains(index)) {
    m_eventConnections[index] = QSharedPointer<OutboundConnections>::create();
  }
  m_eventConnections[index]->connect(peer);
  return true;
}

bool Control::disconnectEvent(int index, const QString &peer, bool entire) {
  auto const &events = m_description->events();
  bool contains = index >= 0 && index < events.size();
  if (!contains) {
    std::stringstream ss;
    ss << "Given index " << index << " is out of range [" << 0 << ", "
       << events.size() << ")";
    throw std::out_of_range(ss.str());
  }
  if (!m_eventConnections.contains(index)) {
    return false;
  }
  if (!m_eventConnections[index]->contains(peer)) {
    return false;
  }
  m_eventConnections[index]->disconnect(peer, entire);
  return true;
}

int Control::refer() { return m_references++; }
int Control::release() { return m_references--; }
int Control::references() { return m_references; }

void Control::slot(const QString &signature, int argc, void *argv) {
  if (!m_eventIndices.contains(signature)) {
    return;
  }
  QSharedPointer<Executor> executor = m_executor;
  if (!executor) {
    return;
  }
  QSharedPointer<OutboundItem> outbound;
  {
    QDateTime timestamp = QDateTime::currentDateTime();
    int index = m_eventIndices[signature];
    QVariantList args = WindowsVariantsToQVariants(argc, argv);
    QSharedPointer<OutboundConnections> connections =
        m_eventConnections.value(index);
    outbound = OutboundItem::create(
        std::move(timestamp), m_uuid, index, std::move(args),
        std::move(connections)
    );
  }
  executor->schedule(outbound);
  while (!outbound->done()) {
    outbound->wait();
    for (const QSharedPointer<InboundItem> &inbound : outbound->receive()) {
      executor->execute(inbound);
    }
  }
}
