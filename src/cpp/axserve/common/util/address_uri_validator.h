#ifndef ADDRESS_URI_VALIDATOR_H
#define ADDRESS_URI_VALIDATOR_H

#include <QValidator>

class AddressURIValidator : public QValidator {
public:
  AddressURIValidator(QObject *parent = nullptr);
  QValidator::State validate(QString &input, int &pos) const override;
  void fixup(QString &input) const override;
};

#endif // ADDRESS_URI_VALIDATOR_H