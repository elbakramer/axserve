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

#ifndef CONTROL_EXTENSION_H
#define CONTROL_EXTENSION_H

#include <vector>

#include <QAxWidget>
#include <QMetaMethod>
#include <QMetaProperty>

#include "util/generic_invoke_method.h"

class ControlExtension {
private:
  QAxWidget *m_control;

  std::vector<QMetaProperty> m_properties;
  std::vector<QMetaMethod> m_methods;
  std::vector<QMetaMethod> m_events;

public:
  explicit ControlExtension(QAxWidget *control)
      : m_control(control) {
    const QMetaObject *meta = m_control->metaObject();
    int prop_offset = meta->propertyOffset();
    int prop_count = meta->propertyCount();
    for (int i = prop_offset; i < prop_count; i++) {
      QMetaProperty prop = meta->property(i);
      if (!prop.isValid()) {
        continue;
      }
      m_properties.push_back(prop);
    }
    int method_offset = meta->methodOffset();
    int method_count = meta->methodCount();
    for (int i = method_offset; i < method_count; i++) {
      QMetaMethod method = meta->method(i);
      if (!method.isValid()) {
        continue;
      }
      QMetaMethod::MethodType method_type = method.methodType();
      if (method_type == QMetaMethod::MethodType::Slot) {
        m_methods.push_back(method);
      } else if (method_type == QMetaMethod::MethodType::Signal) {
        m_events.push_back(method);
      }
    }
  }

  std::vector<QMetaProperty> properties() { return m_properties; }
  std::vector<QMetaMethod> methods() { return m_methods; }
  std::vector<QMetaMethod> events() { return m_events; }

  size_t num_events() { return m_events.size(); }

  int index_of_event(const QString &name) {
    return m_control->metaObject()->indexOfSignal(name.toStdString().c_str()) -
        m_control->metaObject()->methodOffset();
  }

  QVariant get_property(int index) {
    return m_properties.at(index).read(m_control);
  }

  bool set_property(int index, const QVariant &value) {
    return m_properties.at(index).write(m_control, value);
  }

  QVariant invoke_method(
      int index, const QVariantList &args,
      Qt::ConnectionType connection_type = Qt::AutoConnection
  ) {
    return GenericInvokeMethod(
        m_control, m_methods.at(index), args, connection_type
    );
  }
};

#endif
