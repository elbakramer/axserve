#ifndef SHOW_MESSAGE_H
#define SHOW_MESSAGE_H

#include <qstring.h>
#include <qtconfigmacros.h>

QT_BEGIN_NAMESPACE

enum MessageType {
  DebugMessage,
  InfoMessage,
  WarningMessage,
  CriticalMessage,
  FatalMessage,
};

void showMessage(const QString &message, MessageType type);

QT_END_NAMESPACE

#endif // SHOW_MESSAGE_H