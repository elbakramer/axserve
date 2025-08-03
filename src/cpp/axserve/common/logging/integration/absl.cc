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
// SPDX-FileCopyrightText: 2025 Yunseong Hwang
//
// SPDX-License-Identifier: Apache-2.0

#include "axserve/common/logging/integration/absl.h"

#include "spdlog/spdlog.h"

std::string_view
AbslToSpdlogSink::FormatLogMessage(const absl::LogEntry &entry) {
  if (ShouldFormatMessage) {
    return entry.text_message_with_prefix();
  } else {
    return entry.text_message();
  }
}

void AbslToSpdlogSink::Send(const absl::LogEntry &entry) {
  spdlog::level::level_enum level = spdlog::get_level();
  switch (entry.log_severity()) {
  case absl::LogSeverity::kInfo:
    level = spdlog::level::info;
    break;
  case absl::LogSeverity::kWarning:
    level = spdlog::level::warn;
    break;
  case absl::LogSeverity::kError:
    level = spdlog::level::err;
    break;
  case absl::LogSeverity::kFatal:
    level = spdlog::level::critical;
    break;
  }
  spdlog::source_loc loc(
      entry.source_basename().data(), entry.source_line(), nullptr
  );
  spdlog::log(loc, level, FormatLogMessage(entry));
}
