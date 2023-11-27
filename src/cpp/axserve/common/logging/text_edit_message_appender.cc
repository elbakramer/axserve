#include "text_edit_message_appender.h"

PlainTextEditMessageAppender::PlainTextEditMessageAppender(QPlainTextEdit *edit)
    : QObject(edit), m_edit(edit) {}

void PlainTextEditMessageAppender::operator()(
    QtMsgType type, const QMessageLogContext &context, const QString &str
) {
  QString fmt = formatLogMessage(type, context, str);
  m_edit->appendPlainText(fmt);
  m_edit->moveCursor(QTextCursor::StartOfLine);
}