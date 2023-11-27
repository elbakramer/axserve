#ifndef CLSID_VALIDATOR_H
#define CLSID_VALIDATOR_H

#include <QValidator>

class CLSIDValidator : public QValidator {
public:
  CLSIDValidator(QObject *parent = nullptr);
  QValidator::State validate(QString &input, int &pos) const override;
  void fixup(QString &input) const override;
};

#endif // CLSID_VALIDATOR_H