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

#ifndef LOGGING_MANAGER_H
#define LOGGING_MANAGER_H

#include <memory>
#include <string>

#include <QSharedPointer>
#include <QTextEdit>

#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log_sink_registry.h"

#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/qt_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include "axserve/app/model/parsed_config.h"
#include "axserve/common/logging/integration/absl.h"
#include "axserve/common/logging/integration/qt.h"
#include "axserve/common/logging/message_handlers_manager.h"

class LoggingManager {
private:
  std::string file_logging_filename = "log.txt";
  int qt_text_edit_logging_max_lines = 500;

  std::shared_ptr<spdlog::sinks::dist_sink_mt> m_spdlogSink;
  std::shared_ptr<AbslToSpdlogSink> m_abslToSpdlogSink;
  QSharedPointer<QtToSpdlogMessageHandler> m_qtToSpdlogHandler;

  QSharedPointer<QTextEdit> m_editForLogging;

public:
  void setupDefaultLogger();

  void setupConsoleLogging();
  void setupForwardingLogging();

  void setupForwardingAbslLogging();
  void setupForwardingQtLogging();

  void setupBasicFileLogging(const std::string &filename);
  void setupRotatingFileLogging(
      const std::string &filename, std::size_t maxSize, std::size_t maxFiles
  );
  void setupDailyFileLogging(
      const std::string &filename, int hour = 0, int minute = 0,
      std::size_t maxFiles = 0
  );

  void setupQtTextEditLogging(std::size_t maxLines = 500);
  QSharedPointer<QTextEdit> getQtTextEditLogging();

  void setupUsingConfig(const ParsedConfig &config);
};

#endif // LOGGING_MANAGER_H
