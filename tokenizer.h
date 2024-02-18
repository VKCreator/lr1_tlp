#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <QObject>
#include <QDebug>
#include <QRegularExpression>
#include <QtConcurrent>

#include "token.h"

class Tokenizer: public QObject
{
    Q_OBJECT;

public:
    explicit Tokenizer(QString code);

    void lexicalAnalysis();
    QVector<Token>& getTokens() { return tokens; }

private:
    bool nextToken();

private:
    QString code;
    int position = 0;

    QVector<Token> tokens;
    QList<QString> priorityClass;

signals:
    void errorOccurred(int, QString);

};

#endif // TOKENIZER_H
