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

#include "clsid_validator.h"

#include <atlconv.h>
#include <combaseapi.h>

CLSIDValidator::CLSIDValidator(QObject *parent)
    : QValidator(parent){};

QValidator::State CLSIDValidator::validate(QString &input, int &pos) const {
  if (input.trimmed().isEmpty()) {
    return QValidator::Intermediate;
  }
#if defined(_WIN32) && !defined(OLE2ANSI)
  LPCOLESTR lpsz = qUtf16Printable(input);
#else
  LPCOLESTR lpsz = qPrintable(input);
#endif
  CLSID clsid;
  HRESULT res = CLSIDFromString(lpsz, &clsid);
  switch (res) {
  case NOERROR:
    return QValidator::Acceptable;
  case CO_E_CLASSSTRING:
  case E_INVALIDARG:
  default:
    return QValidator::Intermediate;
  }
}

void CLSIDValidator::fixup(QString &input) const { input = input.trimmed(); }
