#include "handle_event_reactors_manager.h"

#include <QtDebug>
#include <QtLogging>

#include <QMutexLocker>

HandleEventReactor *
HandleEventReactorsManager::createReactor(const QString &peer) {
  QMutexLocker<QMutex> lock(&m_reactorsMutex);
  HandleEventReactor *reactor = new HandleEventReactor(sharedFromThis(), peer);
  m_reactors[peer].push_back(reactor);
  return reactor;
}

void HandleEventReactorsManager::deleteReactor(HandleEventReactor *reactor) {
  QMutexLocker<QMutex> lock(&m_reactorsMutex);
  QString peer = reactor->peer();
  m_reactors[peer].removeAll(reactor);
  delete reactor;
  if (m_reactors[peer].size() == 0) {
    m_reactors.remove(peer);
    {
      QMutexLocker<QMutex> lock(&m_connectionsMutex);
      m_connections.remove(peer);
    }
  }
}

void HandleEventReactorsManager::connectEvent(const QString &peer, int index) {
  QMutexLocker<QMutex> lock(&m_connectionsMutex);
  m_connections[peer][index] += 1;
}

void HandleEventReactorsManager::disconnectEvent(
    const QString &peer, int index
) {
  QMutexLocker<QMutex> lock(&m_connectionsMutex);
  m_connections[peer][index] -= 1;
  if (m_connections[peer][index] < 0) {
    m_connections[peer][index] = 0;
  }
  if (m_connections[peer][index] == 0) {
    m_connections[peer].remove(index);
    if (m_connections[peer].size() == 0) {
      m_connections.remove(peer);
      {
        QMutexLocker<QMutex> lock(&m_reactorsMutex);
        m_reactors.remove(peer);
      }
    }
  }
}

void HandleEventReactorsManager::startHandle(
    int index, const QSharedPointer<HandleEventRequest> &request,
    const QSharedPointer<HandleEventCounter> &counter
) {
  QMutexLocker<QMutex> lock1(&m_reactorsMutex);
  QMutexLocker<QMutex> lock2(&m_connectionsMutex);
  counter->setCount(m_reactors.size());
  for (auto [peer, counts] : m_connections.asKeyValueRange()) {
    if (counts[index] > 0) {
      for (auto reactor : m_reactors[peer]) {
        reactor->startHandle(request, counter);
      }
    }
  }
}
