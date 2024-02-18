#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Лексический анализатор");
    setWindowState(Qt::WindowMaximized);

    // Вертикальный сплиттер
    ui->splitter_2->setSizes({2000, 400});

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    ui->textEditCode->setFont(font);

    highlighter = new Highlighter(ui->textEditCode->document());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpenFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                       tr("Открыть"), folder,
                                       tr("C/C++ Files (*.cpp *.h *.c *.hpp)"));
    processFile(fileName);
}

void MainWindow::errorTokenizer(int pos, QString error)
{
    QTextCursor cursor(ui->textEditCode->document());

    cursor.setPosition(pos);

    ui->textEditCode->setTextCursor(cursor);
    QString row = QString::number(ui->textEditCode->textCursor().blockNumber() + 1);
    QString col = QString::number(ui->textEditCode->textCursor().positionInBlock() + 1);

    ui->textEditError->setPlainText(QString("Строка: %1, Колонка: %2, Позиция: %3, Ошибка: %4").arg(row).arg(col).arg(pos).arg(error));

    posError = pos;
    cursor.setPosition(posError, QTextCursor::MoveAnchor);
    cursor.setPosition(posError + 1, QTextCursor::KeepAnchor);

    QTextCharFormat fmt;
    fmt.setBackground(Qt::red);
    cursor.setCharFormat(fmt);

    cursor.setPosition(pos);

    ui->textEditCode->setTextCursor(cursor);
    ui->textEditCode->ensureCursorVisible();

    hasError = true;
}

void MainWindow::on_tableTokens_itemSelectionChanged()
{
    QTextCursor cursor(ui->textEditCode->document());

    if (startPos != -1)
    {
        cursor.setPosition(startPos, QTextCursor::MoveAnchor);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);
        cursor.setCharFormat(QTextCharFormat());

        startPos = -1;
        endPos = -1;
    }

    if (ui->tableTokens->selectedItems().length())
    {
        int row = ui->tableTokens->selectedItems()[0]->row();

        startPos = ui->tableTokens->item(row, POSITION)->text().toInt();
        endPos = startPos + ui->tableTokens->item(row, LENGTH)->text().toInt();

        cursor.setPosition(startPos, QTextCursor::MoveAnchor);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);

        QTextCharFormat fmt;
        fmt.setBackground(Qt::yellow);
        cursor.setCharFormat(fmt);

        cursor.setPosition(startPos);

        ui->textEditCode->setTextCursor(cursor);

        ui->textEditCode->ensureCursorVisible();
    }
}


void MainWindow::processFile(QString fileName)
{
    if (fileName.isEmpty())
        return;

    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
         ui->statusbar->showMessage("Файл не удалось открыть!");
        return;
    }

    QFileInfo info(fileName);

    if (!info.path().startsWith(":"))
        folder = info.path();

    ui->tableTokens->setRowCount(0);

    QTextCursor cursor(ui->textEditCode->document());

    if (posError != -1)
    {
        cursor.setPosition(posError, QTextCursor::MoveAnchor);
        cursor.setPosition(posError + 1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(QTextCharFormat());
        ui->textEditCode->setTextCursor(cursor);

        posError = -1;
    }

    if (startPos != -1)
    {
        cursor.setPosition(startPos, QTextCursor::MoveAnchor);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);
        cursor.setCharFormat(QTextCharFormat());

        startPos = -1;
        endPos = -1;
    }

    ui->textEditError->setText("");
    ui->textEditCode->setPlainText(file.readAll());

    lexer.reset(new Tokenizer(ui->textEditCode->toPlainText()));
    connect(lexer.get(), &Tokenizer::errorOccurred, this, &MainWindow::errorTokenizer);

    hasError = false;
    QFuture<void> future = QtConcurrent::run([this] () {return lexer->lexicalAnalysis();});

    QFutureWatcher<void> watcher;

    QProgressDialog progress("Обработка...", "", 0, 0, this);

    progress.setWindowFlag(Qt::WindowCloseButtonHint, false);
    progress.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    progress.setWindowModality(Qt::WindowModal);
    progress.setCancelButton(nullptr);

    QProgressBar* bar = new QProgressBar(&progress);
    bar->setRange(0,0);
    bar->setValue(0);
    bar->setTextVisible(false);
    progress.setBar(bar);

    connect(&watcher, &QFutureWatcher<QVector<Token*>>::finished, &progress, &QProgressDialog::cancel);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    watcher.setFuture(future);
    progress.exec();

    QApplication::restoreOverrideCursor();

    if (hasError)
        return;

    res = lexer->getTokens();

    for (int var = 0; var < res.length(); ++var)
    {
       QTextCursor cursor(ui->textEditCode->document());

       cursor.setPosition(res[var].getPosition());

       ui->textEditCode->setTextCursor(cursor);

       res[var].setRow(ui->textEditCode->textCursor().blockNumber() + 1);
       res[var].setCol(ui->textEditCode->textCursor().positionInBlock() + 1);
    }

    // Возврат курсора на начало
    QTextCursor cursorStart(ui->textEditCode->document());
    cursorStart.setPosition(QTextCursor::Start);
    ui->textEditCode->setTextCursor(cursorStart);

    //           auto iterRemove = std::remove_if(res.begin(), res.end(), [] (const auto& el) {return el->getType() == "WHITESPACE"; });
    //           res.erase(iterRemove, res.end());
    ui->tableTokens->setRowCount(0);

    for (int var = 0; var < res.length(); ++var)
    {
       ui->tableTokens->insertRow(ui->tableTokens->rowCount());

       ui->tableTokens->setItem(ui->tableTokens->rowCount() - 1, TEXT, new QTableWidgetItem(QString(res[var].getText())));

       ui->tableTokens->setItem(ui->tableTokens->rowCount() - 1, STR, new QTableWidgetItem(QString::number(res[var].getRow())));
       ui->tableTokens->setItem(ui->tableTokens->rowCount() - 1, COL, new QTableWidgetItem(QString::number(res[var].getCol())));
       ui->tableTokens->setItem(ui->tableTokens->rowCount() - 1, POSITION, new QTableWidgetItem(QString::number(res[var].getPosition())));
       ui->tableTokens->setItem(ui->tableTokens->rowCount() - 1, LENGTH, new QTableWidgetItem(QString::number(res[var].getLength())));
       ui->tableTokens->setItem(ui->tableTokens->rowCount() - 1, CLASS, new QTableWidgetItem(QString(res[var].getType())));
    }

    ui->tableTokens->resizeColumnToContents(TEXT);
    ui->tableTokens->resizeColumnToContents(STR);
    ui->tableTokens->resizeColumnToContents(COL);
    ui->tableTokens->resizeColumnToContents(POSITION);
    ui->tableTokens->resizeColumnToContents(LENGTH);
    ui->tableTokens->horizontalHeader()->stretchLastSection();

    ui->textEditError->setPlainText("Ошибок нет.");
//           ui->tableTokens->resizeColumnsToContents();
}

void MainWindow::on_actionHighlighter_triggered()
{
    processFile(":/highlighter.cpp");
}


void MainWindow::on_actionHighlighterH_triggered()
{
    processFile(":/highlighter.h");
}


void MainWindow::on_actionMain_triggered()
{
    processFile(":/main.cpp");
}


void MainWindow::on_actionMainwindow_triggered()
{
    processFile(":/mainwindow.cpp");
}


void MainWindow::on_actionMainwindowH_triggered()
{
   processFile(":/mainwindow.h");
}


void MainWindow::on_actionToken_triggered()
{
    processFile(":/token.cpp");
}


void MainWindow::on_actionTokenH_triggered()
{
    processFile(":/token.h");
}


void MainWindow::on_actionTokenizer_triggered()
{
    processFile(":/tokenizer.cpp");
}


void MainWindow::on_actionTokenizerH_triggered()
{
    processFile(":/tokenizer.h");
}


void MainWindow::on_actionlex_c_triggered()
{
    processFile(":/lex.c");
}


void MainWindow::on_actionexpr_c_triggered()
{
    processFile(":/expr.c");
}


void MainWindow::on_actionqaxserverbase_cpp_triggered()
{
    processFile(":/qaxserverbase.cpp");
}


void MainWindow::on_actionerror_str_cpp_triggered()
{
    processFile(":/error_str.cpp");
}


void MainWindow::on_actionerror_comm_cpp_triggered()
{
    processFile(":/error_comm.cpp");
}


void MainWindow::on_actiondiff_literals_cpp_triggered()
{
    processFile(":/diff_literals.cpp");
}

