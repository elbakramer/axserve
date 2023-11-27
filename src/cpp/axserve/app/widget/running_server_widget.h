#ifndef RUNNING_SERVER_WIDGET_H
#define RUNNING_SERVER_WIDGET_H

#include <Qt>

#include <QPlainTextEdit>
#include <QSharedPointer>
#include <QString>
#include <QWidget>

#include "axserve/common/logging/text_edit_message_appender.h"
#include "axserve/internal/server/active_server.h"

class RunningServerWidget : public QWidget {
  Q_OBJECT

public:
  enum FailedReason {
    NONE,
    CONTROL,
    SERVER,
  };

private:
  QPlainTextEdit *m_edit;
  QSharedPointer<PlainTextEditMessageAppender> m_appender;

  ActiveServer *m_server;

  bool m_isReady;
  FailedReason m_failedReason;

public:
  RunningServerWidget(
      const QString &control, const QString &address, QWidget *parent = nullptr,
      Qt::WindowFlags f = Qt::WindowFlags()
  );

  bool isReady() const;
  FailedReason failedReason() const;
};

#endif // RUNNING_SERVER_WIDGET_H
