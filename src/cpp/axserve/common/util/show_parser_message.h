#ifndef SHOW_PARSER_MESSAGE_H
#define SHOW_PARSER_MESSAGE_H

#include <qstring.h>
#include <qtconfigmacros.h>

QT_BEGIN_NAMESPACE

enum ParserMessageType { UsageMessage, ErrorMessage };

void showParserMessage(const QString &message, ParserMessageType type);

void showParserUsageMessage(const QString &message);
void showParserErrorMessage(const QString &message);

QT_END_NAMESPACE

#endif // SHOW_PARSER_MESSAGE_H