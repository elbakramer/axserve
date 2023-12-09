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

#ifndef START_SERVER_CONFIGURATION_H
#define START_SERVER_CONFIGURATION_H

#include <QString>

class StartServerConfiguration {

private:
  QString m_control;
  QString m_address;

  bool m_createTrayIcon;
  bool m_startHidden;

public:
  StartServerConfiguration();
  StartServerConfiguration(
      const QString &control, const QString &address,
      bool createTrayIcon = false, bool startHidden = false
  );
  StartServerConfiguration(const StartServerConfiguration &other);

  QString control() const;
  QString address() const;

  bool createTrayIcon() const;
  bool startMinimized() const;
  bool startHidden() const;
};

#endif // START_SERVER_CONFIGURATION_H
