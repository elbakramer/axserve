#ifndef HANDLE_EVENT_COUNTER_H
#define HANDLE_EVENT_COUNTER_H

#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QWaitCondition>
#include <QWeakPointer>

class HandleEventCounter {
private:
  int m_id;
  int m_count;
  bool m_started;

  QList<QWeakPointer<QWaitCondition>> m_notificationList;

  QMutex m_countMutex;
  QMutex m_notificationListMutex;

public:
  explicit HandleEventCounter(int id, int count = 0);

  int id();
  int count();

  void setCount(int count);

  void registerNotification(const QSharedPointer<QWaitCondition> &cond);
  void unregisterNotification(const QSharedPointer<QWaitCondition> &cond);

  void handleOne();
  void handleAll();

  bool isDone();
};

#endif