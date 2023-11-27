#ifndef ACTIVE_SERVICE_REQUEST_QUEUE_H
#define ACTIVE_SERVICE_REQUEST_QUEUE_H

#include <QList>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QWeakPointer>

#include "model/active_service_request.h"

class ActiveServiceRequestQueue {
private:
  QQueue<ActiveServiceRequest *> m_queue;
  QList<QWeakPointer<QWaitCondition>> m_notificationList;

  QMutex m_queueMutex;
  QMutex m_notificationListMutex;

public:
  void registerNotification(const QSharedPointer<QWaitCondition> &cond);
  void unregisterNotification(const QSharedPointer<QWaitCondition> &cond);

  void pushRequest(ActiveServiceRequest *request);
  ActiveServiceRequest *popRequest();
};

#endif