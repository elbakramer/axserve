#ifndef VARIANT_CONVERSION_H
#define VARIANT_CONVERSION_H

#include <oaidl.h>

#include <QVariant>

#include "active.pb.h"

bool QVariantToProtoVariant(const QVariant &var, Variant &arg);
QVariant ProtoVariantToQVariant(const Variant &arg);

bool QVariantToWindowsVariant(
    const QVariant &var, VARIANT &arg,
    const QByteArray &typeName = QByteArray(), bool out = false
);
QVariant WindowsVariantToQVariant(
    const VARIANT &arg, const QByteArray &typeName = QByteArray(), int type = 0
);
QVariantList WindowsVariantsToQVariants(
    int argc, void *argv,
    const QList<QByteArray> &typeNames = QList<QByteArray>(),
    const QList<int> &types = QList<int>()
);

#endif // VARIANT_CONVERSION_H
