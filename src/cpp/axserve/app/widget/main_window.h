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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Qt>

#include <QCloseEvent>
#include <QMainWindow>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QWidget>

#include "axserve/app/model/start_server_configuration.h"
#include "axserve/app/widget/running_server_widget.h"
#include "axserve/app/widget/start_server_widget.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(
      QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags()
  );

private:
  QStackedWidget *m_central;
  QSystemTrayIcon *m_trayIcon;

  StartServerWidget *m_start;
  RunningServerWidget *m_running;

protected:
  void closeEvent(QCloseEvent *event) override;

public slots:
  void onInitialStartRequest(const StartServerConfiguration &conf);
  void onStartRequest(const StartServerConfiguration &conf);
  void onTrayIconActivate(QSystemTrayIcon::ActivationReason reason);
};

#endif
