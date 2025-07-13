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

#include "axserve/common/logging/integration/qt.h"

#include "spdlog/spdlog.h"

QString QtToSpdlogMessageHandler::formatLogMessage(
    QtMsgType type, const QMessageLogContext &context, const QString &msg
) {
  if (m_shouldFormatMessage) {
    return AbstractMessageHandler::formatLogMessage(type, context, msg);
  } else {
    return msg;
  }
}

void QtToSpdlogMessageHandler::operator()(
    QtMsgType type, const QMessageLogContext &context, const QString &msg
) {
  spdlog::level::level_enum level = spdlog::get_level();
  switch (type) {
  case QtDebugMsg:
    level = spdlog::level::debug;
    break;
  case QtInfoMsg:
    level = spdlog::level::info;
    break;
  case QtWarningMsg:
    level = spdlog::level::warn;
    break;
  case QtCriticalMsg:
    level = spdlog::level::critical;
    break;
  case QtFatalMsg:
    level = spdlog::level::critical;
    break;
  }
  spdlog::source_loc loc(context.file, context.line, context.function);
  spdlog::log(loc, level, formatLogMessage(type, context, msg).toStdString());
}
