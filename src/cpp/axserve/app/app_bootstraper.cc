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

#include "app_bootstraper.h"

#include <stdio.h>
#include <windows.h>

#include <QApplication>

#include "axserve/app/config.h"

bool hasConsoleWindow() { return GetConsoleWindow() != NULL; }

bool hasUseStdHandlesFlag() {
  STARTUPINFO startupInfo;
  startupInfo.cb = sizeof(STARTUPINFO);
  GetStartupInfo(&startupInfo);
  return startupInfo.dwFlags & STARTF_USESTDHANDLES;
}

bool hasStdHandles() { return _fileno(stdout) > 0 && _fileno(stderr) > 0; }

void AppBootstraper::setupApplicationMetadata() {
  QApplication::setOrganizationName(AXSERVE_ORG_NAME);
  QApplication::setOrganizationDomain(AXSERVE_ORG_DOMAIN);
  QApplication::setApplicationName(AXSERVE_APP_NAME);
  QApplication::setApplicationDisplayName(AXSERVE_APP_DISPLAY_NAME);
  QApplication::setApplicationVersion(AXSERVE_APP_VERSION);
}

bool AppBootstraper::checkConsoleAvailability() {
  bool noConsole =
      !hasConsoleWindow() && !hasUseStdHandlesFlag() && !hasStdHandles();
  QCoreApplication *app = QApplication::instance();
  if (app) {
    app->setProperty("noConsole", noConsole);
  }
  return !noConsole;
}
