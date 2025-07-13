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

#include "control_set.h"

#include "control.h"
#include "executor.h"

ControlSet::ControlSet(const QWeakPointer<Executor> &executor)
    : m_executor(executor) {}

QSharedPointer<Control> ControlSet::create(const QString &c) {
  QSharedPointer<Control> control =
      QSharedPointer<Control>::create(m_executor, c);
  if (!control->initialized()) {
    control.reset();
    return control;
  }
  m_controls[control->instance()] = control;
  return control;
}

bool ControlSet::destroy(const QUuid &i, bool force) {
  bool should_remove = false;
  bool successful = false;
  if (force) {
    should_remove = true;
  } else {
    QSharedPointer<Control> control = find(i);
    should_remove = control->references() <= 0;
  }
  if (should_remove) {
    successful = m_controls.remove(i);
  } else {
    successful = false;
  }
  return successful;
}

bool ControlSet::contains(const QUuid &i) { return m_controls.contains(i); }

QList<QSharedPointer<Control>> ControlSet::list() {
  return m_controls.values();
}

QSharedPointer<Control> ControlSet::find(const QUuid &i) {
  return m_controls.value(i);
}
