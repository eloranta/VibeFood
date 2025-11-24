#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QAbstractItemView>

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
    setupModelAndView();
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

    query.prepare(
        "INSERT OR IGNORE INTO food_ingredients (food_id, ingredient_id, amount) "
        "VALUES ("
        "  (SELECT food_id FROM foods WHERE name = :food),"
        "  (SELECT ingredient_id FROM ingredients WHERE name = :ingredient),"
        "  :amount"
        ");");
    query.bindValue(":food", food);
    query.bindValue(":ingredient", ingredient);
    query.bindValue(":amount", amount);
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

void MainWindow::setupModelAndView()
{
    foodModel = new QSqlTableModel(this, db);
    foodModel->setTable("foods");
    foodModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    foodModel->select();

    foodModel->setHeaderData(1, Qt::Horizontal, "Food");
    foodModel->setHeaderData(2, Qt::Horizontal, "Recipe");

    ui->foodView->setStyleSheet(
        "QTableView::item:selected {"
        "    background-color: #add8ff;"
        "    color: black;"
        "}"
        "QTableView::item:selected:active {"
        "    background-color: #add8ff;"
        "}"
        "QTableView::item:selected:!active {"
        "    background-color: #cce9ff;"
        "}"
        );
    ui->foodView->setModel(foodModel);
    // ui->foodView->hideColumn(0); // food_id
    // ui->foodView->hideColumn(2); // hide recipe column
    ui->foodView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->foodView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->foodView->setSortingEnabled(true);
    ui->foodView->selectRow(0);
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

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS food_ingredients ("
            "  food_id INTEGER NOT NULL,"
            "  ingredient_id INTEGER NOT NULL,"
            "  amount TEXT,"
            "  PRIMARY KEY (food_id, ingredient_id)"
            ");"))
    {
        qWarning() << "Create food_ingredients error:" << query.lastError().text();
        return false;
    }

    if (!query.exec(
            "CREATE VIEW IF NOT EXISTS food_view AS "
            "SELECT f.food_id, f.name AS food_name, fi.amount, i.name AS ingredient_name "
            "FROM food_ingredients fi "
            "JOIN foods f ON fi.food_id = f.food_id "
            "JOIN ingredients i ON fi.ingredient_id = i.ingredient_id "
            "ORDER BY f.name, i.name;"))
    {
        qWarning() << "Create food_view error:" << query.lastError().text();
        return false;
    }

    return true;
}
