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

#ifndef QT_LOGGING_INTEGRATION_H
#define QT_LOGGING_INTEGRATION_H

#include "axserve/common/logging/message_handler.h"

class QtToSpdlogMessageHandler : public AbstractMessageHandler {
private:
  bool m_shouldFormatMessage = false;
  QString formatLogMessage(
      QtMsgType type, const QMessageLogContext &context, const QString &msg
  );

public:
  void operator()(
      QtMsgType type, const QMessageLogContext &context, const QString &msg
  ) override;
};

#endif // QT_LOGGING_INTEGRATION_H
