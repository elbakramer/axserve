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

#include <iostream>

#include <QApplication>

#include "axserve/app/app_bootstraper.h"
#include "axserve/app/command_line_parser.h"
#include "axserve/app/logging_manager.h"

#include "axserve/app/widget/main_window.h"
#include "axserve/app/widget/server_widget.h"
#include "axserve/server/server.h"

#include "axserve/app/config.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  AppBootstraper appBootstraper;

  appBootstraper.setupApplicationMetadata();
  appBootstraper.checkConsoleAvailability();

  LoggingManager loggingManager;

  loggingManager.setupDefaultLogger();
  loggingManager.setupConsoleLogging();
  loggingManager.setupForwardingLogging();
  loggingManager.setupQtTextEditLogging();

  CommandLineParser parser;
  ParsedConfig config = parser.parse(argc, argv);

  if (config.error != nullptr) {
    return config.returnCode;
  }

  loggingManager.setupUsingConfig(config);

  MainWindow mainWindow(config);

  ServerWidget *serverWidget = new ServerWidget(&mainWindow);
  Server *server = new Server(&mainWindow);

  serverWidget->setParsedConfig(config);

  QObject::connect(
      serverWidget, &ServerWidget::startRequested, server, &Server::start
  );
  QObject::connect(
      serverWidget, &ServerWidget::shutdownRequested, server, &Server::shutdown
  );
  QObject::connect(
      server, &Server::statusChanged, serverWidget, &ServerWidget::setStatus
  );

  mainWindow.setCentralWidget(serverWidget);

  if (config.startOnLaunch) {
    server->start(serverWidget->getServerConfig());
  }

  if (config.gui && !config.hideOnLaunch) {
    if (config.minimizeOnLaunch) {
      mainWindow.showMinimized();
    } else {
      mainWindow.showNormal();
    }
  }

  return app.exec();
}
