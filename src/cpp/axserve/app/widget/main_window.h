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
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <QWidget>

#include "axserve/app/model/parsed_config.h"

class MainWindow;

class MainSystemTrayIconMenu : public QMenu {
  Q_OBJECT

private:
  MainWindow *m_main;
  QAction *m_restoreAction;
  QAction *m_exitAction;

public:
  MainSystemTrayIconMenu(MainWindow *main, QWidget *parent = nullptr);
};

class MainSystemTrayIcon : public QSystemTrayIcon {
  Q_OBJECT

private:
  QIcon m_icon;
  MainWindow *m_main;
  MainSystemTrayIconMenu *m_menu;

public:
  MainSystemTrayIcon(MainWindow *main, QObject *parent = nullptr);

public slots:
  void activate(QSystemTrayIcon::ActivationReason reason);
};

class MainWindow : public QMainWindow {
  Q_OBJECT

private:
  QIcon m_icon;
  MainSystemTrayIcon *m_trayIcon;
  ParsedConfig m_config;

public:
  MainWindow(
      ParsedConfig config = {}, QWidget *parent = nullptr,
      Qt::WindowFlags flags = Qt::WindowFlags()
  );

public slots:
  void showRaised();

protected:
  void closeEvent(QCloseEvent *event) override;
  void changeEvent(QEvent *event) override;
};

#endif
