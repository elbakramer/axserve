#include "message_handlers_manager.h"

#include <QtDebug>
#include <QtLogging>

#include <QMutexLocker>

MessageHandlersManager *MessageHandlersManager::m_instance;
QMutex MessageHandlersManager::m_instance_mutex;

MessageHandlersManager::MessageHandlersManager() {
  QtMessageHandler previousHandler = installMessageHandler(messageHandler);
  m_handler_fns.append(previousHandler);
  setupDefaultMessagePattern();
  setupDefaultCategoryFilter();
}

MessageHandlersManager::~MessageHandlersManager() {
  restoreMessageHandler();
  restoreMessagePattern();
  restoreCategoryFilter();
}

MessageHandlersManager *MessageHandlersManager::instance() {
  if (!MessageHandlersManager::m_instance) {
    QMutexLocker lock(&MessageHandlersManager::m_instance_mutex);
    if (!MessageHandlersManager::m_instance) {
      MessageHandlersManager::m_instance = new MessageHandlersManager();
    }
  }
  return MessageHandlersManager::m_instance;
}

void MessageHandlersManager::messageHandler(
    QtMsgType type, const QMessageLogContext &context, const QString &str
) {
  MessageHandlersManager *manager = instance();
  return (*manager)(type, context, str);
}

void MessageHandlersManager::registerHandler(QtMessageHandler handler) {
  QMutexLocker lock(&m_mutex);
  m_handler_fns.append(handler);
}

void MessageHandlersManager::unregisterHandler(QtMessageHandler handler) {
  QMutexLocker lock(&m_mutex);
  m_handler_fns.removeOne(handler);
}

void MessageHandlersManager::registerHandler(
    const QSharedPointer<AbstractMessageHandler> &handler
) {
  QMutexLocker lock(&m_mutex);
  m_handlers.append(handler);
}

void MessageHandlersManager::unregisterHandler(
    const QSharedPointer<AbstractMessageHandler> &handler
) {
  QMutexLocker lock(&m_mutex);
  m_handlers.removeOne(handler);
}

void MessageHandlersManager::operator()(
    QtMsgType type, const QMessageLogContext &context, const QString &str
) {
  QMutexLocker lock(&m_mutex);
  for (auto &handler : m_handler_fns) {
    handler(type, context, str);
  }
  for (auto &maybe_handler : m_handlers) {
    auto handler = maybe_handler.toStrongRef();
    if (handler) {
      (*handler)(type, context, str);
    } else {
      m_handlers.removeAll(handler);
    }
  }
}