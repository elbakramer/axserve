#ifndef HANDLE_EVENT_REACTOR_H
#define HANDLE_EVENT_REACTOR_H

#include <QHash>
#include <QList>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QWeakPointer>

#include "active.grpc.pb.h"

#include "handle_event_counter.h"

using grpc::ServerBidiReactor;

class HandleEventReactorsManager;

class HandleEventReactor
    : public ServerBidiReactor<HandleEventResponse, HandleEventRequest> {
private:
  QWeakPointer<HandleEventReactorsManager> m_manager;
  QString m_peer;

  bool m_done;
  bool m_writing;

  HandleEventResponse m_response;

  QList<QWeakPointer<HandleEventRequest>> m_requests;
  QHash<int, QWeakPointer<HandleEventCounter>> m_counters;

  QMutex m_mutex;

public:
  HandleEventReactor(
      const QSharedPointer<HandleEventReactorsManager> &manager,
      const QString &peer
  );

public:
  QString peer();

  bool startHandle(
      const QSharedPointer<HandleEventRequest> &request,
      const QSharedPointer<HandleEventCounter> &counter
  );

private:
  void NextWrite();

public:
  void OnDone() override;
  void OnReadDone(bool ok) override;
  void OnWriteDone(bool ok) override;
};

#endif
