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

#ifndef COMMAND_LINE_PARSER_H
#define COMMAND_LINE_PARSER_H

#include <string>

#include <CLI/CLI.hpp>
#include <QCoreApplication>

#include "axserve/app/config.h"
#include "axserve/app/model/parsed_config.h"

class CommandLineParser {
  Q_DECLARE_TR_FUNCTIONS(CommandLineParser)

private:
  const std::string appName = AXSERVE_APP_NAME;
  const std::string appVersion = AXSERVE_APP_VERSION;
  const std::string appDescription =
      "AxServe is a server program and client interface that provides "
      "functionalities of COM or OCX components through a gRPC server.";

  CLI::App app;
  ParsedConfig config;

public:
  CommandLineParser();

public:
  ParsedConfig parse(int argc, char *argv[]);
  int exit(const CLI::ParseError &e);
};

#endif // COMMAND_LINE_PARSER_H
