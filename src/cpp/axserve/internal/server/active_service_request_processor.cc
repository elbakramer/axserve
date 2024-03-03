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

#include "active_service_request_processor.h"

#include <sstream>
#include <string>

#include <QtDebug>
#include <QtLogging>

#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMutexLocker>
#include <QVariant>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include "handle_event_counter.h"
#include "util/variant_conversion.h"

using grpc::Status;
using grpc::StatusCode;

using google::protobuf::Message;
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;

QDebug &operator<<(QDebug &os, const Message &msg) {
  std::string s;
  JsonPrintOptions opts;
  opts.preserve_proto_field_names = true;
  absl::Status status = MessageToJsonString(msg, &s, opts);
  if (!status.ok()) {
    std::stringstream ss;
    ss << "Converting message to json string failed.";
    throw std::runtime_error(ss.str());
  }
  os << s.c_str();
  return os;
}

ActiveServiceRequestProcessor::ActiveServiceRequestProcessor(
    QAxWidget *control, QObject *parent
)
    : QObject(parent),
      m_control(control),
      m_control_extension(control),
      m_increment_on_slot(0) {

  m_default_context_request_queue =
      QSharedPointer<ActiveServiceRequestQueue>::create();

  auto num_events = m_control_extension.num_events();

  for (size_t i = 0; i < num_events; i++) {
    m_handle_context_request_queues.append(
        QSharedPointer<ActiveServiceRequestQueue>::create()
    );
  }

  m_reactors_manager = QSharedPointer<HandleEventReactorsManager>::create();
  m_event_context_condition = QSharedPointer<QWaitCondition>::create();

  connect(
      this, &ActiveServiceRequestProcessor::pushed, this,
      &ActiveServiceRequestProcessor::process
  );
  connect(
      m_control, &QAxWidget::signal, this, &ActiveServiceRequestProcessor::slot
  );
}

ServerUnaryReactor *ActiveServiceRequestProcessor::create_unary_reactor(
    CallbackServerContext *context, const DescribeRequest *request,
    DescribeResponse *response
) {
  auto *reactor = context->DefaultReactor();
  auto *service_request =
      new DescribeActiveServiceRequest(context, reactor, request, response);
  push_request(service_request);
  return reactor;
}

ServerUnaryReactor *ActiveServiceRequestProcessor::create_unary_reactor(
    CallbackServerContext *context, const GetPropertyRequest *request,
    GetPropertyResponse *response
) {
  auto *reactor = context->DefaultReactor();
  auto *service_request =
      new GetPropertyActiveServiceRequest(context, reactor, request, response);
  push_request(service_request);
  return reactor;
}

ServerUnaryReactor *ActiveServiceRequestProcessor::create_unary_reactor(
    CallbackServerContext *context, const SetPropertyRequest *request,
    SetPropertyResponse *response
) {
  auto *reactor = context->DefaultReactor();
  auto *service_request =
      new SetPropertyActiveServiceRequest(context, reactor, request, response);
  push_request(service_request);
  return reactor;
}

ServerUnaryReactor *ActiveServiceRequestProcessor::create_unary_reactor(
    CallbackServerContext *context, const InvokeMethodRequest *request,
    InvokeMethodResponse *response
) {
  auto *reactor = context->DefaultReactor();
  auto *service_request =
      new InvokeMethodActiveServiceRequest(context, reactor, request, response);
  push_request(service_request);
  return reactor;
}

ServerUnaryReactor *ActiveServiceRequestProcessor::create_unary_reactor(
    CallbackServerContext *context, const ConnectEventRequest *request,
    ConnectEventResponse *response
) {
  auto *reactor = context->DefaultReactor();
  auto *service_request =
      new ConnectEventActiveServiceRequest(context, reactor, request, response);
  push_request(service_request);
  return reactor;
}

ServerUnaryReactor *ActiveServiceRequestProcessor::create_unary_reactor(
    CallbackServerContext *context, const DisconnectEventRequest *request,
    DisconnectEventResponse *response
) {
  auto *reactor = context->DefaultReactor();
  auto *service_request = new DisconnectEventActiveServiceRequest(
      context, reactor, request, response
  );
  push_request(service_request);
  return reactor;
}

HandleEventReactor *ActiveServiceRequestProcessor::create_handle_reactor(
    CallbackServerContext *context
) {
  QString peer = QString::fromStdString(context->peer());
  auto *reactor = m_reactors_manager->createReactor(peer);
  return reactor;
}

void ActiveServiceRequestProcessor::push_request(ActiveServiceRequest *request
) {
  switch (request->request_context()) {
  case RequestContext::DEFAULT: {
    m_default_context_request_queue->pushRequest(request);
    emit pushed();
    return;
  }
  case RequestContext::EVENT_CALLBACK: {
    m_handle_context_request_queues.at(request->callback_event_index())
        ->pushRequest(request);
    m_event_context_condition->wakeOne();
    return;
  }
  }
}

void ActiveServiceRequestProcessor::process_request(
    ActiveServiceRequest *request
) {
  switch (request->type()) {
  case ActiveServiceRequest::Type::DESCRIBE: {
    DescribeActiveServiceRequest *actual_request =
        static_cast<DescribeActiveServiceRequest *>(request);
    return process_request(actual_request);
  }
  case ActiveServiceRequest::Type::GET_PROPERTY: {
    GetPropertyActiveServiceRequest *actual_request =
        static_cast<GetPropertyActiveServiceRequest *>(request);
    return process_request(actual_request);
  }
  case ActiveServiceRequest::Type::SET_PROPERTY: {
    SetPropertyActiveServiceRequest *actual_request =
        static_cast<SetPropertyActiveServiceRequest *>(request);
    return process_request(actual_request);
  }
  case ActiveServiceRequest::Type::INVOKE_METHOD: {
    InvokeMethodActiveServiceRequest *actual_request =
        static_cast<InvokeMethodActiveServiceRequest *>(request);
    return process_request(actual_request);
  }
  case ActiveServiceRequest::Type::CONNECT_EVENT: {
    ConnectEventActiveServiceRequest *actual_request =
        static_cast<ConnectEventActiveServiceRequest *>(request);
    return process_request(actual_request);
  }
  case ActiveServiceRequest::Type::DISCONNECT_EVENT: {
    DisconnectEventActiveServiceRequest *actual_request =
        static_cast<DisconnectEventActiveServiceRequest *>(request);
    return process_request(actual_request);
  }
  }
}

void ActiveServiceRequestProcessor::process_request(
    DescribeActiveServiceRequest *request
) {
  const auto &properties = m_control_extension.properties();
  for (size_t i = 0; i < properties.size(); i++) {
    const auto &prop = properties.at(i);
    auto prop_info = request->response()->add_properties();
    prop_info->set_index(i);
    prop_info->set_name(prop.name());
    prop_info->set_property_type(prop.typeName());
    prop_info->set_is_readable(prop.isReadable());
    prop_info->set_is_writable(prop.isWritable());
  }
  const auto &methods = m_control_extension.methods();
  for (size_t i = 0; i < methods.size(); i++) {
    const auto &method = methods.at(i);
    auto method_info = request->response()->add_methods();
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
  const auto &events = m_control_extension.events();
  for (size_t i = 0; i < events.size(); i++) {
    const auto &event = events.at(i);
    auto event_info = request->response()->add_events();
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
  return request->reactor()->Finish(Status::OK);
}

void ActiveServiceRequestProcessor::process_request(
    GetPropertyActiveServiceRequest *request
) {
  if (request->context()->IsCancelled()) {
    return request->reactor()->Finish(Status::CANCELLED);
  }
  auto index = request->request()->index();
  QVariant qt_value = m_control_extension.get_property(index);
  bool successful = qt_value.isValid();
  if (successful) {
    Variant &proto_value = *request->response()->mutable_value();
    QVariantToProtoVariant(qt_value, proto_value);
    return request->reactor()->Finish(Status::OK);
  } else {
    Status status(StatusCode::UNKNOWN, "Failed to GetProperty");
    return request->reactor()->Finish(status);
  }
}

void ActiveServiceRequestProcessor::process_request(
    SetPropertyActiveServiceRequest *request
) {
  if (request->context()->IsCancelled()) {
    return request->reactor()->Finish(Status::CANCELLED);
  }
  auto index = request->request()->index();
  QVariant qt_value = ProtoVariantToQVariant(request->request()->value());
  bool successful = m_control_extension.set_property(index, qt_value);
  if (successful) {
    return request->reactor()->Finish(Status::OK);
  } else {
    Status status(StatusCode::UNKNOWN, "Failed to SetProperty");
    return request->reactor()->Finish(status);
  }
}

void ActiveServiceRequestProcessor::process_request(
    InvokeMethodActiveServiceRequest *request
) {
  if (request->context()->IsCancelled()) {
    return request->reactor()->Finish(Status::CANCELLED);
  }
  auto index = request->request()->index();
  QVariantList args;
  for (auto const &item : request->request()->arguments()) {
    QVariant arg = ProtoVariantToQVariant(item);
    args.push_back(std::move(arg));
  }
  QVariant qt_value;
  try {
    qt_value = m_control_extension.invoke_method(index, args);
  } catch (const std::exception &e) {
    Status status(StatusCode::UNKNOWN, e.what());
    return request->reactor()->Finish(status);
  }
  bool successful = qt_value.isValid();
  if (successful) {
    Variant &proto_value = *request->response()->mutable_return_value();
    QVariantToProtoVariant(qt_value, proto_value);
    return request->reactor()->Finish(Status::OK);
  } else {
    Status status(
        StatusCode::OK, "Returning null due to invalid return value."
    );
    return request->reactor()->Finish(status);
  }
}

void ActiveServiceRequestProcessor::process_request(
    ConnectEventActiveServiceRequest *request
) {
  auto peer = QString::fromStdString(request->context()->peer());
  auto index = request->request()->index();
  m_reactors_manager->connectEvent(peer, index);
  request->response()->set_successful(true);
  return request->reactor()->Finish(Status::OK);
}

void ActiveServiceRequestProcessor::process_request(
    DisconnectEventActiveServiceRequest *request
) {
  auto peer = QString::fromStdString(request->context()->peer());
  auto index = request->request()->index();
  try {
    m_reactors_manager->disconnectEvent(peer, index);
  } catch (const std::exception &e) {
    request->response()->set_successful(false);
    Status status(StatusCode::UNKNOWN, e.what());
    return request->reactor()->Finish(status);
  }
  request->response()->set_successful(true);
  return request->reactor()->Finish(Status::OK);
}

void ActiveServiceRequestProcessor::process_default_context_requests() {
  ActiveServiceRequestQueue *queue = m_default_context_request_queue.get();
  while (ActiveServiceRequest *request = queue->popRequest()) {
    process_request(request);
    delete request;
  }
}

void ActiveServiceRequestProcessor::process_handle_context_requests(int index) {
  ActiveServiceRequestQueue *queue =
      m_handle_context_request_queues.at(index).get();
  while (ActiveServiceRequest *request = queue->popRequest()) {
    process_request(request);
    delete request;
  }
}

void ActiveServiceRequestProcessor::process() {
  process_default_context_requests();
}

void ActiveServiceRequestProcessor::slot(
    const QString &name, int argc, void *argv
) {
  QVariantList args = WindowsVariantsToQVariants(argc, argv);

  int event_index = m_control_extension.index_of_event(name);
  int handle_id = m_increment_on_slot++;

  QSharedPointer<HandleEventRequest> request =
      QSharedPointer<HandleEventRequest>::create();
  QSharedPointer<HandleEventCounter> counter =
      QSharedPointer<HandleEventCounter>::create(handle_id);

  counter->registerNotification(m_event_context_condition);

  request->set_id(handle_id);
  request->set_index(event_index);

  for (const auto &qt_arg : args) {
    Variant *proto_arg = request->add_arguments();
    QVariantToProtoVariant(qt_arg, *proto_arg);
  }

  m_reactors_manager->startHandle(event_index, request, counter);

  {
    QMutexLocker<QMutex> lock(&m_event_context_mutex);
    while (!counter->isDone()) {
      m_event_context_condition->wait(&m_event_context_mutex);
      process_handle_context_requests(event_index);
    }
  }
}
