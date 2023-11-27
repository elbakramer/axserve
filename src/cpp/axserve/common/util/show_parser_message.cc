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
    const UINT flags =
        MB_OK | MB_TOPMOST | MB_SETFOREGROUND |
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