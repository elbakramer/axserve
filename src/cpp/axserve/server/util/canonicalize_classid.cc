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
// SPDX-FileCopyrightText: 2025 Yunseong Hwang
//
// SPDX-License-Identifier: Apache-2.0

#include "canonicalize_classid.h"

#include <atlconv.h>
#include <combaseapi.h>

QString CanonicalizeCLSID(const QString &classId) {
#if defined(_WIN32) && !defined(OLE2ANSI)
  LPCOLESTR lpsz = qUtf16Printable(classId);
#else
  LPCOLESTR lpsz = qPrintable(classId);
#endif
  CLSID clsid;
  HRESULT res = CLSIDFromString(lpsz, &clsid);
  switch (res) {
  case NOERROR: {
    LPOLESTR lpsz;
    HRESULT res = StringFromCLSID(clsid, &lpsz);
    switch (res) {
    case S_OK: {
#if defined(_WIN32) && !defined(OLE2ANSI)
      return QString::fromWCharArray(lpsz);
#else
      return QString::fromLocal8Bit(lpsz);
#endif
    }
      // case E_OUTOFMEMORY:
    }
  }
    // case CO_E_CLASSSTRING:
    // case E_INVALIDARG:
  }
  return classId;
}
