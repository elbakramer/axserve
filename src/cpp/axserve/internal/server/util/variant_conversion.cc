// Copyright 2023 Yunseong Hwang
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "variant_conversion.h"

#include <exception>
#include <sstream>

#include <QMetaType>
#include <QString>
#include <QVariant>

#include <QtGlobal>

#define QAXTYPES_P_H                                                           \
<ActiveQt/QT_VERSION_MAJOR.QT_VERSION_MINOR.QT_VERSION_PATCH/ActiveQt/private/qaxtypes_p.h>

#include QAXTYPES_P_H

#include "active.pb.h"

bool QVariantToProtoVariant(const QVariant &var, Variant &arg) {
  if (var.isNull()) {
    return true;
  }
  switch (var.typeId()) {
  case QMetaType::Bool:
    arg.set_bool_value(var.toBool());
    return true;
  case QMetaType::QString:
    arg.set_string_value(var.toString().toStdString());
    return true;
  case QMetaType::Int:
    arg.set_int_value(var.toInt());
    return true;
  case QMetaType::UInt:
    arg.set_uint_value(var.toUInt());
    return true;
  case QMetaType::Double:
    arg.set_double_value(var.toDouble());
    return true;
  case QMetaType::QVariantList: {
    QVariantList in = var.toList();
    VariantList *out = arg.mutable_list_value();
    for (auto const &i : in) {
      Variant &value = *out->add_values();
      QVariantToProtoVariant(i, value);
    }
    return true;
  }
  case QMetaType::QVariantHash: {
    QVariantHash in = var.toHash();
    VaraintHashMap *out = arg.mutable_map_value();
    for (auto i = in.cbegin(), end = in.cend(); i != end; ++i) {
      std::string key = i.key().toStdString();
      Variant &value = (*out->mutable_values())[key];
      QVariantToProtoVariant(i.value(), value);
    }
    return true;
  }
  }
  {
    std::stringstream ss;
    ss << "Cannot convert QVariant with type " << var.typeName()
       << " to Variant";
    throw std::runtime_error(ss.str());
  }
  return false;
}

QVariant ProtoVariantToQVariant(const Variant &arg) {
  switch (arg.value_case()) {
  case Variant::ValueCase::VALUE_NOT_SET:
    return QVariant();
  case Variant::ValueCase::kBoolValue:
    return QVariant(arg.bool_value());
  case Variant::ValueCase::kStringValue:
    return QVariant(QString::fromStdString(arg.string_value()));
  case Variant::ValueCase::kIntValue:
    return QVariant(arg.int_value());
  case Variant::ValueCase::kUintValue:
    return QVariant(arg.uint_value());
  case Variant::ValueCase::kDoubleValue:
    return QVariant(arg.double_value());
  case Variant::ValueCase::kListValue: {
    QVariantList vars;
    auto in = arg.list_value().values();
    for (auto const &i : in) {
      QVariant value = ProtoVariantToQVariant(i);
      vars.append(value);
    }
    return QVariant(vars);
  }
  case Variant::ValueCase::kMapValue: {
    QVariantHash vars;
    auto in = arg.map_value().values();
    for (auto const &i : in) {
      QString key = QString::fromStdString(i.first);
      QVariant value = ProtoVariantToQVariant(i.second);
      vars.insert(key, value);
    }
    return QVariant(vars);
  }
  }
  {
    std::stringstream ss;
    ss << "Cannot convert Variant with type " << arg.value_case()
       << " to QVariant";
    throw std::runtime_error(ss.str());
  }
  return QVariant();
}

bool QVariantToWindowsVariant(
    const QVariant &var, VARIANT &arg, const QByteArray &typeName, bool out
) {
  bool successful = QVariantToVARIANT(var, arg, typeName, out);
  if (!successful) {
    std::stringstream ss;
    ss << "Cannot convert QVariant with type " << var.typeName()
       << " to VARIANT";
    throw std::runtime_error(ss.str());
  }
  return successful;
}

QVariant WindowsVariantToQVariant(
    const VARIANT &arg, const QByteArray &typeName, int type
) {
  QVariant var = VARIANTToQVariant(arg, typeName, type);
  bool successful = var.isValid();
  if (!successful) {
    std::stringstream ss;
    ss << "Cannot convert VARIANT with type " << arg.vt << " to QVariant";
    throw std::runtime_error(ss.str());
  }
  return var;
}

QVariantList WindowsVariantsToQVariants(
    int argc, void *argv, const QList<QByteArray> &typeNames,
    const QList<int> &types
) {
  QVariantList vars;
  VARIANTARG *params = (VARIANTARG *)argv;
  for (int i = 0; i < argc; i++) {
    QVariant arg = WindowsVariantToQVariant(
        params[argc - i - 1], typeNames.value(i), types.value(i)
    );
    vars.push_back(std::move(arg));
  }
  return vars;
}
