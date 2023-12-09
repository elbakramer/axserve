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
