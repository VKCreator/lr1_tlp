#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDebug>
#include <QTableWidgetItem>
#include <QtConcurrent>
#include <QProgressDialog>
#include <QProgressBar>

#include "highlighter.h"
#include "tokenizer.h"

enum ColumnTokens
{
    TEXT,
    STR,
    COL,
    POSITION,
    LENGTH,
    CLASS,

    COUNT_COLS,
};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void processFile(QString fileName);

private slots:
    void errorTokenizer(int pos, QString error);

private slots:
    void on_actionOpenFile_triggered();

    void on_tableTokens_itemSelectionChanged();

    void on_actionHighlighter_triggered();

    void on_actionHighlighterH_triggered();

    void on_actionMain_triggered();

    void on_actionMainwindow_triggered();

    void on_actionMainwindowH_triggered();

    void on_actionToken_triggered();

    void on_actionTokenH_triggered();

    void on_actionTokenizer_triggered();

    void on_actionTokenizerH_triggered();

    void on_actionlex_c_triggered();

    void on_actionexpr_c_triggered();

    void on_actionqaxserverbase_cpp_triggered();

    void on_actionerror_str_cpp_triggered();

    void on_actionerror_comm_cpp_triggered();

    void on_actiondiff_literals_cpp_triggered();

private:
    Ui::MainWindow *ui;

    Highlighter *highlighter;

    QScopedPointer<Tokenizer> lexer;
    QVector<Token> res;

    int startPos = -1;
    int endPos = -1;

    int posError = -1;

    QString folder = ".";

    bool hasError = false;
};
#endif // MAINWINDOW_H
