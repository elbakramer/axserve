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

#ifndef PARSED_CONFIG_H
#define PARSED_CONFIG_H

#include <exception>
#include <string>

struct ParsedConfig {
  std::string addressUri;

  std::string sslRootCertFile;
  std::string sslPrivateKeyFile;
  std::string sslCertChainFile;
  std::string sslClientCertRequestType;

  std::string authApiKeyFile;

  std::string loggingLevel;
  std::string loggingFormat;
  std::string loggingType;
  std::string loggingFile;

  int loggingRotatingMaxSize = 0;
  int loggingRotatingMaxFiles = 0;

  std::string loggingDailyRotatingTime;
  int loggingDailyRotatingHour = 0;
  int loggingDailyRotatingMinute = 0;
  int loggingDailyMaxFiles = 0;

  bool gui = true;
  bool trayIcon = false;
  bool hideOnClose = false;
  bool hideOnMinimize = false;
  bool startOnLaunch = false;
  bool hideOnLaunch = false;
  bool minimizeOnLaunch = false;

  std::string preset;

  std::exception_ptr error;
  int returnCode = 0;
};

#endif // PARSED_CONFIG_H
