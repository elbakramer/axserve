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

#include "address_uri_validator.h"

#include <QHostAddress>
#include <QUrl>

AddressURIValidator::AddressURIValidator(QObject *parent)
    : QValidator(parent){};

QValidator::State
AddressURIValidator::validate(QString &input, int &pos) const {
  QString in = input;
  in = in.trimmed();
  if (in.isEmpty()) {
    return QValidator::Intermediate;
  }
  if (!in.startsWith("dns:///")) {
    in = "dns:///" + in;
  }
  QUrl url = QUrl(in);
  if (!url.isValid()) {
    return QValidator::Intermediate;
  }
  if (url.isLocalFile()) {
    return QValidator::Intermediate;
  }
  return QValidator::Acceptable;
}

void AddressURIValidator::fixup(QString &input) const {
  input = input.trimmed();
  if (!input.isEmpty()) {
    QUrl url = QUrl(input);
    if (url.isValid()) {
      input = url.toDisplayString(
          QUrl::RemoveUserInfo | QUrl::RemovePath | QUrl::RemoveQuery |
          QUrl::RemoveFragment
      );
    }
  }
}
