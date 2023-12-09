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

#include "start_server_configuration.h"

StartServerConfiguration::StartServerConfiguration() {}
StartServerConfiguration::StartServerConfiguration(
    const QString &control, const QString &address, bool createTrayIcon,
    bool startHidden
)
    : m_control(control),
      m_address(address),
      m_createTrayIcon(createTrayIcon),
      m_startHidden(startHidden) {}
StartServerConfiguration::StartServerConfiguration(
    const StartServerConfiguration &other
)
    : StartServerConfiguration(
          other.control(), other.address(), other.createTrayIcon(),
          other.startHidden()
      ) {}

QString StartServerConfiguration::control() const { return m_control; }
QString StartServerConfiguration::address() const { return m_address; }

bool StartServerConfiguration::createTrayIcon() const {
  return m_createTrayIcon;
}
bool StartServerConfiguration::startHidden() const { return m_startHidden; }
