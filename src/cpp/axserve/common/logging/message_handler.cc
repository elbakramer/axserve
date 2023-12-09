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

#include "message_handler.h"

#include <QtDebug>
#include <QtLogging>

#include <QByteArrayView>
#include <QProcessEnvironment>

const QtMessageHandler AbstractMessageHandler::qtDefaultMessageHandler =
    qInstallMessageHandler(0);

const QLoggingCategory::CategoryFilter
    AbstractMessageHandler::qtDefaultCategoryFilter =
        QLoggingCategory::installFilter(nullptr);

const QString AbstractMessageHandler::qtDefaultMessagePattern =
    "%{if-category}%{category}: %{endif}%{message}";

const QString AbstractMessageHandler::appDefaultMessagePattern =
    "%{time yyyy-MM-dd hh:mm:ss.zzz}"
    " [SERVER]"
    " ["
    "%{if-debug}DEBUG%{endif}"
    "%{if-info}INFO%{endif}"
    "%{if-warning}WARNING%{endif}"
    "%{if-critical}CRITICAL%{endif}"
    "%{if-fatal}FATAL%{endif}"
    "] "
    "%{message}";

const QtMsgType AbstractMessageHandler::appDefaultMinimumMessageType =
    QtMsgType::QtDebugMsg;

int AbstractMessageHandler::m_minimumMessageType = -1;

QtMessageHandler
AbstractMessageHandler::installMessageHandler(QtMessageHandler handler) {
  return qInstallMessageHandler(handler);
}

QLoggingCategory::CategoryFilter AbstractMessageHandler::installCategoryFilter(
    QLoggingCategory::CategoryFilter filter
) {
  return QLoggingCategory::installFilter(filter);
}

void AbstractMessageHandler::setMessagePattern(const QString &pattern) {
  return qSetMessagePattern(pattern);
}

QString AbstractMessageHandler::formatLogMessage(
    QtMsgType type, const QMessageLogContext &context, const QString &str
) {
  return qFormatLogMessage(type, context, str);
}

void AbstractMessageHandler::setupDefaultMessagePattern() {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QString qtMessagePattern = env.value("QT_MESSAGE_PATTERN");
  if (qtMessagePattern.isEmpty()) {
    setMessagePattern(appDefaultMessagePattern);
  }
}

void AbstractMessageHandler::setupDefaultCategoryFilter() {
  setMinimumMessageType(appDefaultMinimumMessageType);
}

void AbstractMessageHandler::restoreMessageHandler() {
  installMessageHandler(0);
}
void AbstractMessageHandler::restoreCategoryFilter() {
  installCategoryFilter(nullptr);
}
void AbstractMessageHandler::restoreMessagePattern() {
  setMessagePattern(qtDefaultMessagePattern);
}

bool AbstractMessageHandler::isQtLoggingCategory(QLoggingCategory *category) {
  return QByteArrayView(category->categoryName()).startsWith("qt.");
}

void AbstractMessageHandler::minimumMessageTypeCategoryFilter(
    QLoggingCategory *category
) {
  if (qtDefaultCategoryFilter) {
    qtDefaultCategoryFilter(category);
  }
  if (isQtLoggingCategory(category)) {
    return;
  }
  if (m_minimumMessageType < 0) {
    return;
  }
  switch (m_minimumMessageType) {
  case QtMsgType::QtDebugMsg:
    category->setEnabled(QtMsgType::QtDebugMsg, true);
    category->setEnabled(QtMsgType::QtInfoMsg, true);
    category->setEnabled(QtMsgType::QtWarningMsg, true);
    category->setEnabled(QtMsgType::QtCriticalMsg, true);
    break;
  case QtMsgType::QtInfoMsg:
    category->setEnabled(QtMsgType::QtDebugMsg, false);
    category->setEnabled(QtMsgType::QtInfoMsg, true);
    category->setEnabled(QtMsgType::QtWarningMsg, true);
    category->setEnabled(QtMsgType::QtCriticalMsg, true);
    break;
  case QtMsgType::QtWarningMsg:
    category->setEnabled(QtMsgType::QtDebugMsg, false);
    category->setEnabled(QtMsgType::QtInfoMsg, false);
    category->setEnabled(QtMsgType::QtWarningMsg, true);
    category->setEnabled(QtMsgType::QtCriticalMsg, true);
    break;
  case QtMsgType::QtCriticalMsg:
    category->setEnabled(QtMsgType::QtDebugMsg, false);
    category->setEnabled(QtMsgType::QtInfoMsg, false);
    category->setEnabled(QtMsgType::QtWarningMsg, false);
    category->setEnabled(QtMsgType::QtCriticalMsg, true);
    break;
  case QtMsgType::QtFatalMsg:
    category->setEnabled(QtMsgType::QtDebugMsg, false);
    category->setEnabled(QtMsgType::QtInfoMsg, false);
    category->setEnabled(QtMsgType::QtWarningMsg, false);
    category->setEnabled(QtMsgType::QtCriticalMsg, false);
    break;
  }
}

int AbstractMessageHandler::getMinimumMessageType() {
  return m_minimumMessageType;
}

int AbstractMessageHandler::setMinimumMessageType(QtMsgType type) {
  int previousMinimumType = m_minimumMessageType;
  m_minimumMessageType = type;
  installCategoryFilter(minimumMessageTypeCategoryFilter);
  return previousMinimumType;
}
