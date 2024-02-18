#include "tokentype.h"

TokenType::TokenType(QString name, QRegularExpression regex)
    : name(name), regex(regex)
{

}
