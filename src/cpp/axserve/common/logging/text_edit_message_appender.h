#ifndef TEXT_EDIT_MESSAGE_APPENDER_H
#define TEXT_EDIT_MESSAGE_APPENDER_H

#include <QObject>
#include <QPlainTextEdit>

#include "message_handler.h"

class PlainTextEditMessageAppender : public QObject,
                                     public AbstractMessageHandler {
  Q_OBJECT

private:
  QPlainTextEdit *m_edit;

public:
  PlainTextEditMessageAppender(QPlainTextEdit *edit);

  void operator()(
      QtMsgType type, const QMessageLogContext &context, const QString &str
  ) override;
};

#endif // TEXT_EDIT_MESSAGE_APPENDER_H