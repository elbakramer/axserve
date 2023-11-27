#include "active_service_request_queue.h"

#include <QMutexLocker>

void ActiveServiceRequestQueue::registerNotification(
    const QSharedPointer<QWaitCondition> &cond
) {
  QMutexLocker<QMutex> lock(&m_notificationListMutex);
  m_notificationList.append(cond);
}

void ActiveServiceRequestQueue::unregisterNotification(
    const QSharedPointer<QWaitCondition> &cond
) {
  QMutexLocker<QMutex> lock(&m_notificationListMutex);
  m_notificationList.removeOne(cond);
}

void ActiveServiceRequestQueue::pushRequest(ActiveServiceRequest *request) {
  {
    QMutexLocker<QMutex> lock(&m_queueMutex);
    m_queue.append(request);
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

ActiveServiceRequest *ActiveServiceRequestQueue::popRequest() {
  QMutexLocker<QMutex> lock(&m_queueMutex);
  ActiveServiceRequest *request = nullptr;
  if (!m_queue.empty()) {
    request = m_queue.first();
    m_queue.removeFirst();
  }
  return request;
}
