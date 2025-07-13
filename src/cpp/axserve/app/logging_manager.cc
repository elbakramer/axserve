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

#include "logging_manager.h"

void LoggingManager::setupDefaultLogger() {
  spdlog::init_thread_pool(8192, 1);
  m_spdlogSink = std::make_shared<spdlog::sinks::dist_sink_mt>();
  auto logger = std::make_shared<spdlog::async_logger>(
      "default", m_spdlogSink, spdlog::thread_pool(),
      spdlog::async_overflow_policy::block
  );
  spdlog::set_default_logger(logger);
}

void LoggingManager::setupConsoleLogging() {
  if (!m_spdlogSink) {
    setupDefaultLogger();
  }
  auto consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  m_spdlogSink->add_sink(consoleSink);
}

void LoggingManager::setupForwardingLogging() {
  setupForwardingAbslLogging();
  setupForwardingQtLogging();
}

void LoggingManager::setupForwardingAbslLogging() {
  absl::InitializeLog();
  absl::SetStderrThreshold(absl::LogSeverityAtLeast::kFatal);
  m_abslToSpdlogSink = std::make_shared<AbslToSpdlogSink>();
  absl::AddLogSink(m_abslToSpdlogSink.get());
}

void LoggingManager::setupForwardingQtLogging() {
  MessageHandlersManager::instance();
  m_qtToSpdlogHandler = QSharedPointer<QtToSpdlogMessageHandler>::create();
  MessageHandlersManager::instance()->registerHandler(m_qtToSpdlogHandler);
}

void LoggingManager::setupBasicFileLogging(const std::string &filename) {
  if (!m_spdlogSink) {
    setupDefaultLogger();
  }
  auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
  m_spdlogSink->add_sink(fileSink);
}

void LoggingManager::setupRotatingFileLogging(
    const std::string &filename, std::size_t maxSize, std::size_t maxFiles
) {}

void LoggingManager::setupDailyFileLogging(
    const std::string &filename, int hour, int minute, std::size_t maxFiles
) {
  if (!m_spdlogSink) {
    setupDefaultLogger();
  }
  auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
      filename, hour, minute, false, maxFiles
  );
  m_spdlogSink->add_sink(fileSink);
}

void LoggingManager::setupQtTextEditLogging(std::size_t maxLines) {
  if (!m_spdlogSink) {
    setupDefaultLogger();
  }
  m_editForLogging = QSharedPointer<QTextEdit>::create();
  auto qtSink = std::make_shared<spdlog::sinks::qt_color_sink_mt>(
      m_editForLogging.get(), maxLines
  );
  m_spdlogSink->add_sink(qtSink);
}

QSharedPointer<QTextEdit> LoggingManager::getQtTextEditLogging() {
  return m_editForLogging;
}

void LoggingManager::setupUsingConfig(const ParsedConfig &config) {
  if (config.loggingType.starts_with("file")) {
    if (config.loggingType == "file" || config.loggingType == "file-basic") {
      setupBasicFileLogging(config.loggingFile);
    } else if (config.loggingType == "file-rotating") {
      setupRotatingFileLogging(
          config.loggingFile, config.loggingRotatingMaxSize,
          config.loggingRotatingMaxFiles
      );
    } else if (config.loggingType == "file-daily") {
      setupDailyFileLogging(
          config.loggingFile, config.loggingDailyRotatingHour,
          config.loggingDailyRotatingMinute, config.loggingDailyMaxFiles
      );
    }
  }
}
