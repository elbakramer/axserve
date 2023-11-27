#ifndef HANDLE_EVENT_REACTORS_MANAGER_H
#define HANDLE_EVENT_REACTORS_MANAGER_H

#include <QEnableSharedFromThis>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QString>

#include "active.pb.h"

#include "handle_event_counter.h"
#include "handle_event_reactor.h"

class HandleEventReactorsManager
    : public QEnableSharedFromThis<HandleEventReactorsManager> {
private:
  QHash<QString, QList<HandleEventReactor *>> m_reactors;
  QMutex m_reactorsMutex;

  QHash<QString, QHash<int, int>> m_connections;
  QMutex m_connectionsMutex;

public:
  HandleEventReactor *createReactor(const QString &peer);
  void deleteReactor(HandleEventReactor *reactor);

  void connectEvent(const QString &peer, int index);
  void disconnectEvent(const QString &peer, int index);

  void startHandle(
      int index, const QSharedPointer<HandleEventRequest> &request,
      const QSharedPointer<HandleEventCounter> &counter
  );
};

#endif
