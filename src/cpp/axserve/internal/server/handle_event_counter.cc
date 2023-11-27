#include "handle_event_counter.h"

#include <QMutexLocker>

HandleEventCounter::HandleEventCounter(int id, int count)
    : m_id(id), m_count(count), m_started(false) {}

int HandleEventCounter::id() { return m_id; }

int HandleEventCounter::count() {
  QMutexLocker<QMutex> lock(&m_countMutex);
  return m_count;
}

void HandleEventCounter::setCount(int count) {
  QMutexLocker<QMutex> lock(&m_countMutex);
  if (m_started) {
    return;
  }
  m_count = count;
}

void HandleEventCounter::registerNotification(
    const QSharedPointer<QWaitCondition> &cond
) {
  QMutexLocker<QMutex> lock(&m_notificationListMutex);
  m_notificationList.append(cond);
}

void HandleEventCounter::unregisterNotification(
    const QSharedPointer<QWaitCondition> &cond
) {
  QMutexLocker<QMutex> lock(&m_notificationListMutex);
  m_notificationList.removeOne(cond);
}

void HandleEventCounter::handleOne() {
  {
    QMutexLocker<QMutex> lock(&m_countMutex);
    m_started = true;
    m_count -= 1;
    if (m_count < 0) {
      m_count = 0;
    }
  }
  {
    QMutexLocker<QMutex> lock(&m_notificationListMutex);
    for (const auto &wc : m_notificationList) {
      auto sc = wc.toStrongRef();
      if (sc) {
        sc->wakeOne();
      } else {
        m_notificationList.removeAll(wc);
      }
    }
  }
}

void HandleEventCounter::handleAll() {
  {
    QMutexLocker<QMutex> lock(&m_countMutex);
    m_started = true;
    m_count = 0;
  }
  {
    QMutexLocker<QMutex> lock(&m_notificationListMutex);
    for (const auto &wc : m_notificationList) {
      auto sc = wc.toStrongRef();
      if (sc) {
        sc->wakeAll();
      } else {
        m_notificationList.removeAll(wc);
      }
    }
  }
}

bool HandleEventCounter::isDone() {
  QMutexLocker<QMutex> lock(&m_countMutex);
  return m_count == 0;
}