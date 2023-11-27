#ifndef MESSAGE_HANDLERS_MANAGER_H
#define MESSAGE_HANDLERS_MANAGER_H

#include <functional>

#include <QtLogging>

#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QWeakPointer>

#include "message_handler.h"

class MessageHandlersManager : public AbstractMessageHandler {
private:
  static MessageHandlersManager *m_instance;
  static QMutex m_instance_mutex;

private:
  QMutex m_mutex;
  QList<QtMessageHandler> m_handler_fns;
  QList<QWeakPointer<AbstractMessageHandler>> m_handlers;

private:
  MessageHandlersManager();

public:
  virtual ~MessageHandlersManager();

public:
  static MessageHandlersManager *instance();
  static void messageHandler(
      QtMsgType type, const QMessageLogContext &context, const QString &str
  );

  void registerHandler(QtMessageHandler handler);
  void unregisterHandler(QtMessageHandler handler);

  void registerHandler(const QSharedPointer<AbstractMessageHandler> &handler);
  void unregisterHandler(const QSharedPointer<AbstractMessageHandler> &handler);

  void operator()(
      QtMsgType type, const QMessageLogContext &context, const QString &str
  ) override;
};

#endif // MESSAGE_HANDLERS_MANAGER_H