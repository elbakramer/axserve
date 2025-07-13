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

#include "executor.h"

#include <exception>
#include <sstream>

#include "control.h"
#include "control_set.h"
#include "inbound_channel.h"
#include "inbound_item.h"
#include "outbound_channel.h"
#include "outbound_item.h"
#include "outbound_reactor_set.h"

#include "util/variant_conversion.h"

#include "axserve/common/logging/protobuf_message_logging.h"

Executor::Executor() {}

QSharedPointer<Executor> Executor::create() {
  QSharedPointer<Executor> executor = QSharedPointer<Executor>::create();
  executor->initialize();
  return executor;
}

void Executor::initialize() {
  QSharedPointer<Executor> executor = sharedFromThis();
  if (executor) {
    m_controls = QSharedPointer<ControlSet>::create(executor);
    m_inbounds = QSharedPointer<InboundChannel>::create(executor);
    m_outbounds = QSharedPointer<OutboundChannel>::create(executor);
  }
  m_reactors = QSharedPointer<OutboundReactorSet>::create();
  connect(this, &Executor::inbound, this, &Executor::handleInbound);
  connect(this, &Executor::outbound, this, &Executor::handleOutbound);
}

bool Executor::addControl(const QString &classId) {
  return !m_controls->create(classId).isNull();
}

void Executor::schedule(const QSharedPointer<InboundItem> &item) {
  ContextType context = item->contextType();
  switch (context) {
  case ContextType::DEFAULT: {
    m_inbounds->send(item);
    break;
  }
  case ContextType::EVENT: {
    m_outbounds->send(item);
    break;
  }
  default: {
    std::stringstream ss;
    ss << "Unexpected request context: " << context;
    throw std::runtime_error(ss.str());
  }
  }
}

void Executor::notifyInbound() { emit inbound(); }

void Executor::handleInbound() {
  QQueue<QSharedPointer<InboundItem>> items = m_inbounds->receive();
  for (const QSharedPointer<InboundItem> &item : items) {
    execute(item);
  }
}

void Executor::schedule(const QSharedPointer<OutboundItem> &item) {
  m_outbounds->send(item);
}

void Executor::notifyOutbound() { emit outbound(); }

void Executor::handleOutbound() {
  QQueue<QSharedPointer<OutboundItem>> items = m_outbounds->receive();
  for (const QSharedPointer<OutboundItem> &item : items) {
    execute(item);
  }
}

bool Executor::execute(const QSharedPointer<OutboundItem> &item) {
  if (item->canceled()) {
    return false;
  }
  m_reactors->send(item);
  return true;
}

bool Executor::execute(const QSharedPointer<InboundItem> &item) {
  if (item->canceled()) {
    return false;
  }
  switch (item->type()) {
  case InboundItem::Type::CREATE:
    return execute(item.staticCast<CreateInboundItem>());
  case InboundItem::Type::REFER:
    return execute(item.staticCast<ReferInboundItem>());
  case InboundItem::Type::RELEASE:
    return execute(item.staticCast<ReleaseInboundItem>());
  case InboundItem::Type::DESTROY:
    return execute(item.staticCast<DestroyInboundItem>());
  case InboundItem::Type::LIST:
    return execute(item.staticCast<ListInboundItem>());
  case InboundItem::Type::DESCRIBE:
    return execute(item.staticCast<DescribeInboundItem>());
  case InboundItem::Type::GET_PROPERTY:
    return execute(item.staticCast<GetPropertyInboundItem>());
  case InboundItem::Type::SET_PROPERTY:
    return execute(item.staticCast<SetPropertyInboundItem>());
  case InboundItem::Type::INVOKE_METHOD:
    return execute(item.staticCast<InvokeMethodInboundItem>());
  case InboundItem::Type::CONNECT_EVENT:
    return execute(item.staticCast<ConnectEventInboundItem>());
  case InboundItem::Type::DISCONNECT_EVENT:
    return execute(item.staticCast<DisconnectEventInboundItem>());
  default:
    qWarning() << "Unexpected inbound item type:" << item->type();
  }
  return false;
}

bool Executor::execute(const QSharedPointer<CreateInboundItem> &item) {
  item->start();
  QString clsid = QString::fromStdString(item->request()->clsid());
  QSharedPointer<Control> control = m_controls->create(clsid);
  if (!control) {
    Status status(StatusCode::UNKNOWN, "Failed to create an instance");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  item->response()->set_instance(control->instance().toString().toStdString());
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<ReferInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool contains = m_controls->contains(uuid);
  if (!contains) {
    Status status(StatusCode::UNKNOWN, "Target instance does not exist");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  QSharedPointer<Control> control = m_controls->find(uuid);
  control->refer();
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<ReleaseInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool contains = m_controls->contains(uuid);
  if (!contains) {
    Status status(StatusCode::UNKNOWN, "Target instance does not exist");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  QSharedPointer<Control> control = m_controls->find(uuid);
  control->release();
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<DestroyInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool successful = m_controls->destroy(uuid, true);
  if (!successful) {
    Status status(StatusCode::UNKNOWN, "Failed to destroy the instance");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  item->response()->set_successful(successful);
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<ListInboundItem> &item) {
  item->start();
  QList<QSharedPointer<Control>> controls = m_controls->list();
  for (const QSharedPointer<Control> &control : controls) {
    ListItem *e = item->response()->add_items();
    QUuid u = control->instance();
    QString c = control->control();
    int r = control->references();
    e->set_instance(u.toString().toStdString());
    e->set_clsid(c.toStdString());
    e->set_references(r);
  }
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<DescribeInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool contains = m_controls->contains(uuid);
  if (!contains) {
    Status status(StatusCode::UNKNOWN, "Target instance does not exist");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  QSharedPointer<Control> control = m_controls->find(uuid);
  const Control::Description &description = control->describe();
  for (size_t i = 0; i < description.properties().size(); i++) {
    const auto &prop = description.properties().at(i);
    auto prop_info = item->response()->add_properties();
    prop_info->set_index(i);
    prop_info->set_name(prop.name());
    prop_info->set_property_type(prop.typeName());
    prop_info->set_is_readable(prop.isReadable());
    prop_info->set_is_writable(prop.isWritable());
  }
  for (size_t i = 0; i < description.methods().size(); i++) {
    const auto &method = description.methods().at(i);
    auto method_info = item->response()->add_methods();
    method_info->set_index(i);
    method_info->set_name(method.name().toStdString());
    auto param_count = method.parameterCount();
    auto param_names = method.parameterNames();
    for (int j = 0; j < param_count; j++) {
      auto arg = method_info->add_arguments();
      arg->set_name(param_names.at(j).toStdString());
      arg->set_argument_type(method.parameterTypeName(j));
    }
    method_info->set_return_type(method.typeName());
  }
  for (size_t i = 0; i < description.events().size(); i++) {
    const auto &event = description.events().at(i);
    auto event_info = item->response()->add_events();
    event_info->set_index(i);
    event_info->set_name(event.name().toStdString());
    auto param_count = event.parameterCount();
    auto param_names = event.parameterNames();
    for (int j = 0; j < param_count; j++) {
      auto arg = event_info->add_arguments();
      arg->set_name(param_names.at(j).toStdString());
      arg->set_argument_type(event.parameterTypeName(j));
    }
  }
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<GetPropertyInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool contains = m_controls->contains(uuid);
  if (!contains) {
    Status status(StatusCode::UNKNOWN, "Target instance does not exist");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  QSharedPointer<Control> control = m_controls->find(uuid);
  int index = item->request()->index();
  QVariant qt_value;
  try {
    qt_value = control->getProperty(index);
  } catch (const std::exception &e) {
    Status status(StatusCode::UNKNOWN, e.what());
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  bool successful = qt_value.isValid();
  if (!successful) {
    Status status(StatusCode::UNKNOWN, "Failed to get property");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  Variant &proto_value = *item->response()->mutable_value();
  QVariantToProtoVariant(qt_value, proto_value);
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<SetPropertyInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool contains = m_controls->contains(uuid);
  if (!contains) {
    Status status(StatusCode::UNKNOWN, "Target instance does not exist");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  QSharedPointer<Control> control = m_controls->find(uuid);
  int index = item->request()->index();
  QVariant qt_value = ProtoVariantToQVariant(item->request()->value());
  bool successful = false;
  try {
    successful = control->setProperty(index, std::move(qt_value));
  } catch (const std::exception &e) {
    Status status(StatusCode::UNKNOWN, e.what());
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  if (!successful) {
    Status status(StatusCode::UNKNOWN, "Failed to set property");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<InvokeMethodInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool contains = m_controls->contains(uuid);
  if (!contains) {
    Status status(StatusCode::UNKNOWN, "Target instance does not exist");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  QSharedPointer<Control> control = m_controls->find(uuid);
  int index = item->request()->index();
  QVariantList args;
  for (auto const &a : item->request()->arguments()) {
    QVariant arg = ProtoVariantToQVariant(a);
    args.push_back(std::move(arg));
  }
  QVariant qt_value;
  try {
    qt_value = control->invokeMethod(index, args);
  } catch (const std::exception &e) {
    Status status(StatusCode::UNKNOWN, e.what());
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  bool successful = qt_value.isValid();
  if (!successful) {
    Status status(
        StatusCode::OK, "Returning null due to invalid return value."
    );
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  Variant &proto_value = *item->response()->mutable_return_value();
  QVariantToProtoVariant(qt_value, proto_value);
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<ConnectEventInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool contains = m_controls->contains(uuid);
  if (!contains) {
    Status status(StatusCode::UNKNOWN, "Target instance does not exist");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  QSharedPointer<Control> control = m_controls->find(uuid);
  int index = item->request()->index();
  QString peer = item->peer();
  bool successful = false;
  try {
    successful = control->connectEvent(index, peer);
  } catch (const std::exception &e) {
    Status status(StatusCode::UNKNOWN, e.what());
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  if (!successful) {
    Status status(StatusCode::UNKNOWN, "Failed to connect event");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  item->response()->set_successful(true);
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

bool Executor::execute(const QSharedPointer<DisconnectEventInboundItem> &item) {
  item->start();
  QUuid uuid = QUuid::fromString(item->request()->instance());
  bool contains = m_controls->contains(uuid);
  if (!contains) {
    Status status(StatusCode::UNKNOWN, "Target instance does not exist");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  QSharedPointer<Control> control = m_controls->find(uuid);
  int index = item->request()->index();
  QString peer = item->peer();
  bool successful = false;
  try {
    successful = control->disconnectEvent(index, peer);
  } catch (const std::exception &e) {
    Status status(StatusCode::UNKNOWN, e.what());
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  if (!successful) {
    Status status(StatusCode::UNKNOWN, "Failed to disconnect event");
    item->reactor()->Finish(status);
    item->finish();
    return false;
  }
  item->response()->set_successful(true);
  item->reactor()->Finish(Status::OK);
  item->finish();
  return true;
}

void Executor::establish(const QSharedPointer<OutboundReactor> &reactor) {
  return m_reactors->establish(reactor);
}

bool Executor::dissolve(const QSharedPointer<OutboundReactor> &reactor) {
  return m_reactors->dissolve(reactor);
}
