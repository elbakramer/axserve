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

#include "generic_invoke_method.h"

#include <exception>
#include <sstream>

#include <Qt>

#include <QMetaMethod>
#include <QObject>
#include <QVariant>

QVariant GenericInvokeMethod(
    QObject *object, const QMetaMethod &method, const QVariantList &args,
    Qt::ConnectionType type
) {
  return GenericInvokeMethod_New(object, method, args, type);
}

QVariant GenericInvokeMethod_Old(
    QObject *object, const QMetaMethod &method, const QVariantList &args,
    Qt::ConnectionType type
) {
  QVariantList convertedArgs;

  if (args.size() < method.parameterCount()) {
    std::stringstream ss;
    ss << "Insufficient arguments (" << args.size() << " < "
       << method.parameterCount() << ") to call"
       << method.methodSignature().toStdString();
    throw std::runtime_error(ss.str());
  }

  for (int i = 0; i < method.parameterCount(); i++) {
    const QVariant &arg = args.at(i);

    QMetaType argType = arg.metaType();
    QMetaType methodArgType = method.parameterMetaType(i);

    QVariant copy = QVariant(arg);

    if (argType != methodArgType) {
      if (copy.canConvert(methodArgType)) {
        if (!copy.convert(methodArgType)) {
          std::stringstream ss;
          ss << "Cannot convert args[" << i << "] from" << argType.name()
             << "to" << methodArgType.name();
          throw std::runtime_error(ss.str());
        }
      }
    }

    convertedArgs << copy;
  }

  QList<QGenericArgument> genericArguments;

  for (int i = 0; i < convertedArgs.size(); i++) {
    QVariant &arg = convertedArgs[i];
    QMetaType argType = arg.metaType();
    QGenericArgument genericArgument(argType.name(), arg.constData());
    genericArguments << genericArgument;
  }

  QVariant returnValue(method.returnMetaType());
  QMetaType returnType = method.returnMetaType();
  QGenericReturnArgument genericReturnArgument(
      returnType.name(), returnValue.data()
  );

  bool successful = method.invoke(
      object, type, genericReturnArgument, genericArguments.value(0),
      genericArguments.value(1), genericArguments.value(2),
      genericArguments.value(3), genericArguments.value(4),
      genericArguments.value(5), genericArguments.value(6),
      genericArguments.value(7), genericArguments.value(8),
      genericArguments.value(9)
  );

  if (!successful) {
    std::stringstream ss;
    ss << "Calling" << method.methodSignature().toStdString() << "failed.";
    throw std::runtime_error(ss.str());
  } else {
    return returnValue;
  }
}

QVariant GenericInvokeMethod_New(
    QObject *object, const QMetaMethod &method, const QVariantList &args,
    Qt::ConnectionType type
) {
  QVariantList convertedArgs;

  if (args.size() < method.parameterCount()) {
    std::stringstream ss;
    ss << "Insufficient arguments (" << args.size() << " < "
       << method.parameterCount() << ") to call"
       << method.methodSignature().toStdString();
    throw std::runtime_error(ss.str());
  }

  for (int i = 0; i < method.parameterCount(); i++) {
    const QVariant &arg = args.at(i);

    QMetaType argType = arg.metaType();
    QMetaType methodArgType = method.parameterMetaType(i);

    QVariant copy = QVariant(arg);

    if (argType != methodArgType) {
      if (copy.canConvert(methodArgType)) {
        if (!copy.convert(methodArgType)) {
          std::stringstream ss;
          ss << "Cannot convert args[" << i << "] from" << argType.name()
             << "to" << methodArgType.name();
          throw std::runtime_error(ss.str());
        }
      }
    }

    convertedArgs << copy;
  }

  QList<QMetaMethodArgument> genericArguments;

  for (int i = 0; i < convertedArgs.size(); i++) {
    QVariant &arg = convertedArgs[i];
    QMetaType argType = arg.metaType();
    QMetaMethodArgument genericArgument = {
        argType.iface(), argType.name(), arg.constData()
    };
    genericArguments << genericArgument;
  }

  QVariant returnValue(method.returnMetaType());
  QMetaType returnType = method.returnMetaType();
  QMetaMethodReturnArgument genericReturnArgument = {
      returnType.iface(), returnType.name(), returnValue.data()
  };

  bool successful = false;

  switch (method.parameterCount()) {
  case 0:
    successful = method.invoke(object, type, genericReturnArgument);
    break;
  case 1:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0)
    );
    break;
  case 2:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1)
    );
    break;
  case 3:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1), genericArguments.value(2)
    );
    break;
  case 4:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1), genericArguments.value(2),
        genericArguments.value(3)
    );
    break;
  case 5:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1), genericArguments.value(2),
        genericArguments.value(3), genericArguments.value(4)
    );
    break;
  case 6:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1), genericArguments.value(2),
        genericArguments.value(3), genericArguments.value(4),
        genericArguments.value(5)
    );
    break;
  case 7:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1), genericArguments.value(2),
        genericArguments.value(3), genericArguments.value(4),
        genericArguments.value(5), genericArguments.value(6)
    );
    break;
  case 8:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1), genericArguments.value(2),
        genericArguments.value(3), genericArguments.value(4),
        genericArguments.value(5), genericArguments.value(6),
        genericArguments.value(7)
    );
    break;
  case 9:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1), genericArguments.value(2),
        genericArguments.value(3), genericArguments.value(4),
        genericArguments.value(5), genericArguments.value(6),
        genericArguments.value(7), genericArguments.value(8)
    );
    break;
  case 10:
    successful = method.invoke(
        object, type, genericReturnArgument, genericArguments.value(0),
        genericArguments.value(1), genericArguments.value(2),
        genericArguments.value(3), genericArguments.value(4),
        genericArguments.value(5), genericArguments.value(6),
        genericArguments.value(7), genericArguments.value(8),
        genericArguments.value(9)
    );
    break;
  }

  if (!successful) {
    std::stringstream ss;
    ss << "Calling" << method.methodSignature().toStdString() << "failed.";
    throw std::runtime_error(ss.str());
  } else {
    return returnValue;
  }
}
