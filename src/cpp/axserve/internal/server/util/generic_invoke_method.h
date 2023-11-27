#ifndef GENERIC_INVOKE_METHOD_H
#define GENERIC_INVOKE_METHOD_H

#include <Qt>

#include <QMetaMethod>
#include <QObject>
#include <QVariant>

QVariant GenericInvokeMethod(
    QObject *object, const QMetaMethod &method, const QVariantList &args,
    Qt::ConnectionType type = Qt::AutoConnection
);

QVariant GenericInvokeMethod_Old(
    QObject *object, const QMetaMethod &method, const QVariantList &args,
    Qt::ConnectionType type = Qt::AutoConnection
);

QVariant GenericInvokeMethod_New(
    QObject *object, const QMetaMethod &method, const QVariantList &args,
    Qt::ConnectionType type = Qt::AutoConnection
);

#endif // GENERIC_INVOKE_METHOD_H