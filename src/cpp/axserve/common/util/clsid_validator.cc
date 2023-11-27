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