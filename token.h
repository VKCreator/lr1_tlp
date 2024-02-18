#ifndef TOKEN_H
#define TOKEN_H

#include <QString>

class Token
{

public:
    explicit Token(QString type, QString text, int position);

    QString getType() const { return type; }
    QString getText() const { return text; }
    int getLength() const { return length; }
    int getPosition() const { return position; }
    int getRow() const { return str; }
    int getCol() const { return col; }

    void setType(QString type){ this->type = type; }
    void setLength(int length) { this->length = length; }
    void setPosition(int position) { this->position = position; }
    void setRow(int row) { this->str = row; }
    void setCol(int col) { this->col = col; }
    void setText(QString text) { this->text = text; }

private:
    QString type;
    QString text;
    int position; // in all source code
    int str;
    int col;
    int length;
};

#endif // TOKEN_H
