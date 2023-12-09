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

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <functional>

#include <QtLogging>

#include <QLoggingCategory>
#include <QString>

typedef std::function<
    void(QtMsgType, const QMessageLogContext &, const QString &)>
    QtMessageHandlerFn;

class AbstractMessageHandler {
public:
  static const QtMessageHandler qtDefaultMessageHandler;
  static const QLoggingCategory::CategoryFilter qtDefaultCategoryFilter;
  static const QString qtDefaultMessagePattern;

public:
  static const QString appDefaultMessagePattern;
  static const QtMsgType appDefaultMinimumMessageType;

public:
  static QtMessageHandler installMessageHandler(QtMessageHandler handler);
  static QLoggingCategory::CategoryFilter
  installCategoryFilter(QLoggingCategory::CategoryFilter filter);

  static void setMessagePattern(const QString &pattern);
  static QString formatLogMessage(
      QtMsgType type, const QMessageLogContext &context, const QString &str
  );

public:
  static void setupDefaultMessagePattern();
  static void setupDefaultCategoryFilter();

  static void restoreMessageHandler();
  static void restoreMessagePattern();
  static void restoreCategoryFilter();

private:
  static int m_minimumMessageType;

public:
  static int getMinimumMessageType();
  static int setMinimumMessageType(QtMsgType type);

  static bool isQtLoggingCategory(QLoggingCategory *category);
  static void minimumMessageTypeCategoryFilter(QLoggingCategory *category);

public:
  virtual void operator()(
      QtMsgType type, const QMessageLogContext &context, const QString &str
  ) = 0;
};

#endif // MESSAGE_HANDLER_H
