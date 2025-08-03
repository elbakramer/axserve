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
 * SPDX-FileCopyrightText: 2025 Yunseong Hwang
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ABSL_LOGGING_INTEGRATION_H
#define ABSL_LOGGING_INTEGRATION_H

#include "absl/log/log_sink.h"

class AbslToSpdlogSink : public absl::LogSink {
private:
  bool ShouldFormatMessage = false;
  std::string_view FormatLogMessage(const absl::LogEntry &entry);

public:
  void Send(const absl::LogEntry &entry) override;
};

#endif // ABSL_LOGGING_INTEGRATION_H
