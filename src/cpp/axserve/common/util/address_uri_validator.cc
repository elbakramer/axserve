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