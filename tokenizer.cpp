#include "tokenizer.h"

/*
 * Token:
 * indentifier
 * keyword
 * literal (strings and character and numeric)
 * operator
 * punctuator
*/

// комментарии

QHash<QString, QRegularExpression> tokensType =
{
    {"IDENT", QRegularExpression("^[_a-zA-Z][_a-zA-Z0-9]*")},

    {"KEYWORDS", QRegularExpression("^(alignas|continue|friend|register|true|alignof|decltype|goto|reinterpret_cast|try|asm|"
     "default|if|return|typedef|auto|delete|inline|short|typeid|bool|double|int|signed|"
     "typename|break|do|long|sizeof union|case|dynamic_cast|mutable|static|unsigned|catch|else|namespace|"
     "static_assert|using|char|enum|new|static_cast|virtual|char16_t|explicit|noexcept|struct|void|char32_t|export|"
     "nullptr|switch|volatile|class|extern|operator|template|wchar_t|const|false|private|this|while|constexpr|float|protected|thread_local"
     "|const_cast|for|public|throw)\\b")},

    {"STRING_LITERAL", QRegularExpression("^\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"")},

    {"DECIMAL_LITERAL", QRegularExpression("^[-+]?(?:0|[1-9](?:_*[0-9])*)[uU]?[lL]?")},
    {"FLOAT_LITERAL", QRegularExpression("^[+-]?([0-9]+\\.[0-9]+)")},

    {"OCTAL_LITERAL", QRegularExpression("(^0[0-7](?:_*[0-7])*[uU]?[lL]?)")},

    {"BINARY_LITERAL", QRegularExpression("^0b[01][01]*[uU]?[lL]?", QRegularExpression::CaseInsensitiveOption)},

    {"HEX_LITERAL", QRegularExpression("(^0x[a-fA-F0-9](?:_*[a-fA-F0-9])*[uU]?[lL]?)", QRegularExpression::CaseInsensitiveOption)},

    {"CHAR_LITERAL", QRegularExpression("^'([^'\\\\\n]|\\\\.)'")},

    {"UNARY_OPERATORS", QRegularExpression("^(\\+\\+|--|::|->|\\.|\\[|\\]|\\(|\\)|~|!|-|\\+|&|\\*|<|>)")},

    {"BINARY_OPERATORS_ARITHMETIC", QRegularExpression("^(\\*|\\/|%|\\+|-)")},
    {"BINARY_OPERATORS_COMPARISON", QRegularExpression("^(<=|>=|<|>|==|!=)")},
    {"BINARY_OPERATORS_LOGIC", QRegularExpression("^(&&|\\|\\|)")},
    {"BINARY_OPERATORS_BITWISE", QRegularExpression("^(&|\\^|\\||<<|>>)")},
    {"BINARY_OPERATORS_ASSIGN", QRegularExpression("^(=|\\*=|\\/=|%=|\\+=|-=|<<=|>>=|&=|\\|=|\\^=)")},
    {"BINARY_OPERATORS_COMMA", QRegularExpression("^(\\,)")},

    {"TERNARY_OPERATOR", QRegularExpression("^(\\?)")},

    {"WHITESPACE", QRegularExpression("^(\\s|\\n|\\t\\r)")},
    {"PUNCT", QRegularExpression("^(,|;|{|}|\\(|\\)|\\.\\.\\.|\\.|\\\\|:)")},

    {"SINGLE_LINE_COMMENT", QRegularExpression("^\\/\\/.*")},
    {"MULTILINE_COMMENT", QRegularExpression("^(\\/\\*)([\\s\\S]*?)(\\*\\/)")},
    {"MULTILINE_COMMENT_NO_CLOSE", QRegularExpression("^(\\/\\*)")},

    {"DIRECTIVES", QRegularExpression("^#(\\s*include|ifndef|undef|if|else|endif|define|elif|)")},
};


Tokenizer::Tokenizer(QString code)
    : code(code)
{
    priorityClass.append("WHITESPACE");

    priorityClass.append("KEYWORDS");

    priorityClass.append("IDENT");

    priorityClass.append("STRING_LITERAL");

    priorityClass.append("FLOAT_LITERAL");
    priorityClass.append("DECIMAL_LITERAL");
    priorityClass.append("OCTAL_LITERAL");
    priorityClass.append("BINARY_LITERAL");
    priorityClass.append("HEX_LITERAL");
    priorityClass.append("CHAR_LITERAL");

    priorityClass.append("UNARY_OPERATORS");

    priorityClass.append("TERNARY_OPERATOR");

    priorityClass.append("BINARY_OPERATORS_ARITHMETIC");
    priorityClass.append("BINARY_OPERATORS_COMPARISON");
    priorityClass.append("BINARY_OPERATORS_LOGIC");
    priorityClass.append("BINARY_OPERATORS_BITWISE");
    priorityClass.append("BINARY_OPERATORS_ASSIGN");
    priorityClass.append("BINARY_OPERATORS_COMMA");

    priorityClass.append("PUNCT");

    priorityClass.append("SINGLE_LINE_COMMENT");
    priorityClass.append("MULTILINE_COMMENT");
    priorityClass.append("MULTILINE_COMMENT_NO_CLOSE");

    priorityClass.append("DIRECTIVES");
}

void Tokenizer::lexicalAnalysis()
{
    try
    {
        while (nextToken()) {}
    }
    catch(std::pair<int, QString>& exc)
    {
        emit errorOccurred(exc.first, exc.second);
    }
}

bool Tokenizer::nextToken()
{
    if (position >= code.length())
        return false;

    QVector<Token> tmp;
    QVector<int> length;
    tmp.clear();
    length.clear();

    for (int i = 0; i < priorityClass.length(); ++i)
    {
        auto type = priorityClass[i];
        auto regex = tokensType[type];

       // qDebug() << code.mid(position);

        QRegularExpressionMatch match = regex.match(code.mid(position));
        if (match.hasMatch())
        {
            if (type == "WHITESPACE")
            {
                position += (match.captured(0).length());
                return true;
            }

            Token tok(type, match.captured(0), position);
            tok.setLength(match.captured(0).length());
            tmp.append(tok);
            length.append(match.captured(0).length());

            if (type == "KEYWORDS")
                break;

//            return true;
        }
    }

    int maxV = *std::max_element(length.begin(), length.end());
    QString type = "";
    int maxInd = 0;

    for (int var = 0; var < tmp.length(); ++var)
    {
        if (length[var] == maxV)
        {
            maxInd = var;
            type = type + (type.length() > 0 ? " | " : "") + tmp[var].getType();
        }
    }

    if (!tmp.empty())
        tokens.append(tmp[maxInd]);

    if (!tmp.empty())
    {
        if (type == "MULTILINE_COMMENT_NO_CLOSE")
            throw std::make_pair(position, QString("Незакрытый многострочный комментарий."));

        tokens[tokens.length() - 1].setType(type);
        tokens[tokens.length() - 1].setPosition(position);
        position += (tokens[tokens.length() - 1].getText().length());

//        qDebug() << tokens[tokens.length() - 1]->getText();

        if (type == "SINGLE_LINE_COMMENT")
            tokens[tokens.length() - 1].setText("//");

        if (type == "MULTILINE_COMMENT")
            tokens[tokens.length() - 1].setText("/* */");

        return true;
    }

    throw std::make_pair(position, QString("Токен не совпал ни с одним классом."));
}
