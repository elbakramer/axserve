/*
 * Copyright 2023 Yunseong Hwang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CONTROL_H
#define CONTROL_H

#include <Qt>

#include <QAtomicInteger>
#include <QAxWidget>
#include <QDateTime>
#include <QHash>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QVariantList>
#include <QVector>
#include <QWeakPointer>

class Executor;
class OutboundConnections;

class Control : public QObject {
  Q_OBJECT

public:
  class Description {
  private:
    friend class Control;

  private:
    const QVector<QMetaProperty> m_properties;
    const QVector<QMetaMethod> m_methods;
    const QVector<QMetaMethod> m_events;

  private:
    Description();
    Description(
        QVector<QMetaProperty> &&properties, QVector<QMetaMethod> &&methods,
        QVector<QMetaMethod> &&events
    );

  public:
    const QVector<QMetaProperty> &properties() const;
    const QVector<QMetaMethod> &methods() const;
    const QVector<QMetaMethod> &events() const;
  };

private:
  QWeakPointer<Executor> m_executor;
  QUuid m_uuid;
  QString m_clsid;
  QScopedPointer<QAxWidget> m_control;
  QScopedPointer<Description> m_description;
  QHash<QString, int> m_eventIndices;
  QHash<int, QSharedPointer<OutboundConnections>> m_eventConnections;
  QAtomicInteger<int> m_references;

public:
  Control(const QWeakPointer<Executor> &executor, const QString &c);

  bool initialized();

  const QUuid &instance();
  const QString &control();

  const Description &describe();

  QVariant getProperty(int index);
  bool setProperty(int index, const QVariant &value);
  bool setProperty(int index, QVariant &&value);
  QVariant invokeMethod(
      int index, const QVariantList &args,
      Qt::ConnectionType connection_type = Qt::AutoConnection
  );
  bool connectEvent(int index, const QString &peer);
  bool disconnectEvent(int index, const QString &peer, bool entire = false);

  int refer();
  int release();
  int references();

private slots:
  void slot(const QString &signature, int argc, void *argv);
};

#endif // CONTROL_H
