#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if (!openDatabase())
    {
        QMessageBox::critical(this, "Database error", "Failed to open SQLite database.");
        return;
    }

    if (!createTables())
    {
        QMessageBox::critical(this, "Database error", "Failed to create tables.");
        return;
    }

    seedDataIfEmpty();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openDatabase()
{
    const QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("vibefood.db");

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open())
    {
        qWarning() << "DB open error:" << db.lastError().text();
        return false;
    }

    return true;
}

void MainWindow::addItem(const QString &food, const QString &ingredient, const QString &amount)
{
    QSqlQuery query;

    query.prepare("INSERT OR IGNORE INTO foods (name) VALUES (:n)");
    query.bindValue(":n", food);
    query.exec();

    query.prepare("INSERT OR IGNORE INTO ingredients (name) VALUES (:n)");
    query.bindValue(":n", ingredient);
    query.exec();
}

void MainWindow::seedDataIfEmpty()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM food_ingredients;");
    if (query.next() && query.value(0).toInt() != 0)
        return;

    addItem("Tortilla", "Tortilla lettuja", "");
    addItem("Tortilla", "Jauhelihaa", "400 g");
    addItem("Tortilla", "Valkosipuli", "1 kynsi");
    addItem("Tortilla", "Tomaatti", "");
    addItem("Tortilla", "Kurkku", "");
    addItem("Tortilla", "Pakastemaissi", "");
    addItem("Tortilla", "Turkkilainen jogurtti", "");

    addItem("Lohikeitto", "Lohi", "");
    addItem("Lohikeitto", "Peruna", "");
    addItem("Lohikeitto", "Porkkana", "");
    addItem("Lohikeitto", "Sipuli", "");
    addItem("Lohikeitto", "Voi", "");
    addItem("Lohikeitto", "Kuohukerma", "");
    addItem("Lohikeitto", "Tilli", "");

    addItem("Paistettu maksa", "Maksa", "1 pc");
    addItem("Paistettu maksa", "Voi", "1 pc");

    addItem("Spagetti Bolognese", "Pancetta", "");
    addItem("Spagetti Bolognese", "Sipuli", "");
    addItem("Spagetti Bolognese", "Porkkana", "");
    addItem("Spagetti Bolognese", "Selleri", "");
    addItem("Spagetti Bolognese", "Jauheliha", "");
    addItem("Spagetti Bolognese", "Punaviini", "");
    addItem("Spagetti Bolognese", "Tomaattipyree", "");
    addItem("Spagetti Bolognese", "Paseerattu tomaatti", "");
    addItem("Spagetti Bolognese", "Lihaliemi", "");
    addItem("Spagetti Bolognese", "Maito", "");
    addItem("Spagetti Bolognese", "Spagetti", "");

    addItem("Graavilohi", "Lohi", "");

    addItem("Hampurilainen", "Jauheliha", "400 g");
    addItem("Hampurilainen", "Juustoviipaleet", "4");
    addItem("Hampurilainen", "Sämpylä", "4");
    addItem("Hampurilainen", "Majoneesi", "");
    addItem("Hampurilainen", "Jäävuorisalaatti", "");

    addItem("Sienipannu", "Sieni", "");
    addItem("Sienipannu", "Valkosipuli", "");
    addItem("Sienipannu", "Pecorino", "");
    addItem("Sienipannu", "Aurajuusto", "");

    addItem("Lohi nigiri", "Lohi", "");
    addItem("Lohi nigiri", "Riisi", "");
    addItem("Lohi nigiri", "Teriyaki kastike", "");
    addItem("Lohi nigiri", "Majoneesi", "");
    addItem("Lohi nigiri", "Wasabi", "");
    addItem("Lohi nigiri", "Seesamin siemenet", "");
    addItem("Lohi nigiri", "Ruohosipuli", "");

    addItem("Makaronilaatikko", "Makaroni", "");
    addItem("Makaronilaatikko", "Jauheliha", "");
    addItem("Makaronilaatikko", "Muna", "");
    addItem("Makaronilaatikko", "Maito", "");
    addItem("Makaronilaatikko", "Juusto", "");

    addItem("Ribsit", "Porsaan ribsit", "");
    addItem("Ribs", "BBQ-kastike", "");

    addItem("Nyhtöpossu", "Ribsit", "");
    addItem("Nyhtöpossu", "BBQ-kastike", "");
    addItem("Nyhtöpossu", "Soijakastike", "");
}
bool MainWindow::createTables()
{
    QSqlQuery query(db);

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS foods ("
            "  food_id INTEGER PRIMARY KEY,"
            "  name TEXT NOT NULL UNIQUE,"
            "  recipe TEXT"
            ");"))
    {
        qWarning() << "Create foods error:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS ingredients ("
            "  ingredient_id INTEGER PRIMARY KEY,"
            "  name TEXT NOT NULL UNIQUE"
            ");"))
    {
        qWarning() << "Create ingredients error:" << query.lastError().text();
        return false;
    }

    return true;
}
