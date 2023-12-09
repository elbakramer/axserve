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

#include "main_window.h"

#include <Qt>
#include <QtDebug>
#include <QtLogging>

#include <QAction>
#include <QCoreApplication>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QStackedWidget>
#include <QString>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QWidget>

#include "axserve/app/config.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags) {

  // create and set central widget
  m_central = new QStackedWidget(this);
  setCentralWidget(m_central);

  // add start server widget as default central widget
  m_start = new StartServerWidget(this);
  m_central->addWidget(m_start);
  m_central->setCurrentWidget(m_start);
  connect(
      m_start, &StartServerWidget::startRequested, this,
      &MainWindow::onStartRequest
  );

  // create system tray icon
  m_trayIcon = new QSystemTrayIcon(this);
  QMenu *menu = new QMenu(this);
  QAction *exitAction = menu->addAction(tr("Exit"));
  m_trayIcon->setContextMenu(menu);
  connect(
      exitAction, &QAction::triggered, QCoreApplication::instance(),
      &QCoreApplication::quit, Qt::QueuedConnection
  );
  connect(
      m_trayIcon, &QSystemTrayIcon::activated, this,
      &MainWindow::onTrayIconActivate
  );

  // use builtin Qt icon for both tray icon and window icon
  QStyle::StandardPixmap standardIcon =
      QStyle::StandardPixmap::SP_TitleBarMenuButton;
  QIcon icon = style()->standardIcon(standardIcon);
  m_trayIcon->setIcon(icon);
  setWindowIcon(icon);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (!event->spontaneous() || !isVisible())
    return;
  if (m_trayIcon->isVisible()) {
    hide();
    m_trayIcon->showMessage(
        tr("Window is currently hidden."),
        tr("Double click tray icon to reopen the window.")
    );
    event->ignore();
  }
}

void MainWindow::onInitialStartRequest(const StartServerConfiguration &conf) {
  // pass initial values to edits
  m_start->onInitialStartRequest(conf);

  // check gui availability
  bool noGui = QCoreApplication::instance()
      ? QCoreApplication::instance()->property("noGui").toBool()
      : false;

  // show main window if needed
  if (!noGui && !conf.startHidden()) {
    showNormal();
  }

  // try start if possible
  if (!conf.control().isEmpty() && !conf.address().isEmpty()) {
    if (!m_start->onStartButtonClick()) {
      if (noGui) {
        ::exit(EXIT_FAILURE);
      } else if (!isVisible()) {
        showNormal();
      }
    }
  }
}

void MainWindow::onStartRequest(const StartServerConfiguration &conf) {
  qDebug() << "Starting server with clsid:" << conf.control()
           << "and address:" << conf.address();

  RunningServerWidget *running =
      new RunningServerWidget(conf.control(), conf.address(), this);

  if (!running->isReady()) {
    // check gui availability and app title for printing errors
    QCoreApplication *app = QCoreApplication::instance();
    bool noGui = false;
    QString title;
    if (app)
      noGui = app->property("noGui").toBool();
    if (!noGui) {
      if (app)
        title = app->property("applicationDisplayName").toString();
      if (title.isEmpty()) {
        title = QCoreApplication::applicationName();
      }
    }

    // print error based on failed reason
    switch (running->failedReason()) {
    case RunningServerWidget::FailedReason::CONTROL: {
      QString message = tr("Failed to initialize COM/OCX for the given CLSID.");
      qCritical() << qPrintable(message);
      if (!noGui) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(title);
        msgBox.setText(message);
        msgBox.exec();
      }
      if (noGui) {
        ::exit(EXIT_FAILURE);
      } else if (!isVisible()) {
        showNormal();
      }
      break;
    }
    case RunningServerWidget::FailedReason::SERVER: {
      QString message =
          tr("Failed to start server, possibly due to the invalid address "
             "given.");
      qCritical() << qPrintable(message);
      if (!noGui) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(title);
        msgBox.setText(message);
        msgBox.exec();
      }
      if (noGui) {
        ::exit(EXIT_FAILURE);
      } else if (!isVisible()) {
        showNormal();
      }
      break;
    }
    default: {
      QString message = tr("Failed to start server, for unknown reason.");
      qCritical() << qPrintable(message);
      if (!noGui) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(title);
        msgBox.setText(message);
        msgBox.exec();
      }
      if (noGui) {
        ::exit(EXIT_FAILURE);
      } else if (!isVisible()) {
        showNormal();
      }
      break;
    }
    }

    // delete invalid running widget
    running->deleteLater();
  } else {
    // add successful values to edit history
    m_start->addLineEditHistory(conf.control(), conf.address());

    // add and set running widget as current widget
    m_running = running;
    m_central->addWidget(running);
    m_central->setCurrentWidget(running);

    // create tray icon if needed
    if (conf.createTrayIcon()) {
      m_trayIcon->show();
      m_trayIcon->setToolTip(conf.control());
    }

    // minimize or hide if needed
    if (isVisible() && m_trayIcon->isVisible() && conf.startHidden()) {
      hide();
    }
  }
}

void MainWindow::onTrayIconActivate(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
  case QSystemTrayIcon::DoubleClick: {
    if (!isVisible()) {
      showNormal();
    }
  }
  }
}
