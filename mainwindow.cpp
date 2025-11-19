#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if (!openDatabase()) {
        QMessageBox::critical(this, "Database error", "Failed to open SQLite database.");
        return;
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openDatabase()
{
    // Put the DB file next to the executable
    QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("vibefood.db");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open())
    {
        qWarning() << "DB open error:" << db.lastError().text();
        return false;
    }

    return true;
}


