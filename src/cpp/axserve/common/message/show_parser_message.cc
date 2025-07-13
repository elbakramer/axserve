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

#include "show_parser_message.h"

#include <qcoreapplication.h>
#include <qlatin1stringview.h>
#include <qstring.h>
#include <qsystemdetection.h>
#include <qtconfigmacros.h>
#include <qvariant.h>
#if defined(Q_OS_WIN) && !defined(QT_BOOTSTRAPPED)
#include <qt_windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

#if defined(Q_OS_WIN) && !defined(QT_BOOTSTRAPPED)
// Return whether to use a message box. Use handles if a console can be obtained
// or we are run with redirected handles (for example, by QProcess).
static inline bool displayMessageBox() {
  if (GetConsoleWindow() ||
      qEnvironmentVariableIsSet("QT_COMMAND_LINE_PARSER_NO_GUI_MESSAGE_BOXES"))
    return false;
  STARTUPINFO startupInfo;
  startupInfo.cb = sizeof(STARTUPINFO);
  GetStartupInfo(&startupInfo);
  return !(startupInfo.dwFlags & STARTF_USESTDHANDLES);
}
#endif // Q_OS_WIN && !QT_BOOTSTRAPPED

void showParserMessage(const QString &message, ParserMessageType type) {
#if defined(Q_OS_WIN) && !defined(QT_BOOTSTRAPPED)
  if (displayMessageBox()) {
    const UINT flags = MB_OK | MB_TOPMOST | MB_SETFOREGROUND |
        (type == UsageMessage ? MB_ICONINFORMATION : MB_ICONERROR);
    QString title;
    if (QCoreApplication::instance())
      title = QCoreApplication::instance()
                  ->property("applicationDisplayName")
                  .toString();
    if (title.isEmpty())
      title = QCoreApplication::applicationName();
    MessageBoxW(
        0, reinterpret_cast<const wchar_t *>(message.utf16()),
        reinterpret_cast<const wchar_t *>(title.utf16()), flags
    );
    return;
  }
#endif // Q_OS_WIN && !QT_BOOTSTRAPPED
  fputs(qPrintable(message), type == UsageMessage ? stdout : stderr);
}

void showParserUsageMessage(const QString &message) {
  showParserMessage(
      QCoreApplication::applicationName() + ": "_L1 + message + u'\n',
      UsageMessage
  );
}

void showParserErrorMessage(const QString &message) {
  showParserMessage(
      QCoreApplication::applicationName() + ": "_L1 + message + u'\n',
      ErrorMessage
  );
}

QT_END_NAMESPACE
