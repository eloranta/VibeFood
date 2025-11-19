#include "mainwindow.h"

#include <QTableView>
#include <QHeaderView>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRelationalDelegate>
#include <QMessageBox>
#include <QDebug>
#include <QVariant>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    model(nullptr),
    tableView(new QTableView(this))
{
    setWindowTitle("VibeFood");

    if (!setupDatabase()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to open database. The app will show nothing.");
    } else {
        initData();
        setupModelAndView();
    }

    setCentralWidget(tableView);
    resize(800, 400);
}

MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close();
}

bool MainWindow::setupDatabase()
{
    // SQLite database file in working directory (next to exe)
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("vibefood.db");

    if (!db.open()) {
        qWarning() << "DB open error:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    // Create categories table
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS categories ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL UNIQUE"
            ");")) {
        qWarning() << "Create categories error:" << query.lastError().text();
        return false;
    }

    // Create foods table WITHOUT calories
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS foods ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL,"
            "  category_id INTEGER,"
            "  FOREIGN KEY(category_id) REFERENCES categories(id)"
            ");")) {
        qWarning() << "Create foods error:" << query.lastError().text();
        return false;
    }

    return true;
}

void MainWindow::initData()
{
    QSqlQuery query(db);

    // ---- Seed categories if empty ----
    if (!query.exec("SELECT COUNT(*) FROM categories;")) {
        qWarning() << "Count categories error:" << query.lastError().text();
        return;
    }
    if (query.next() && query.value(0).toInt() == 0) {
        if (!query.exec("INSERT INTO categories (name) VALUES ('Fruit');"))
            qWarning() << "Insert category error:" << query.lastError().text();
        if (!query.exec("INSERT INTO categories (name) VALUES ('Fast Food');"))
            qWarning() << "Insert category error:" << query.lastError().text();
        if (!query.exec("INSERT INTO categories (name) VALUES ('Drink');"))
            qWarning() << "Insert category error:" << query.lastError().text();
    }

    // ---- Seed foods if empty ----
    if (!query.exec("SELECT COUNT(*) FROM foods;")) {
        qWarning() << "Count foods error:" << query.lastError().text();
        return;
    }
    if (query.next() && query.value(0).toInt() == 0) {

        int fruitId = -1, fastId = -1, drinkId = -1;

        QSqlQuery catQuery(db);
        if (!catQuery.exec("SELECT id, name FROM categories;")) {
            qWarning() << "Select categories error:" << catQuery.lastError().text();
            return;
        }

        while (catQuery.next()) {
            const QString name = catQuery.value(1).toString();
            int id = catQuery.value(0).toInt();
            if (name == "Fruit") {
                fruitId = id;
            } else if (name == "Fast Food") {
                fastId = id;
            } else if (name == "Drink") {
                drinkId = id;
            }
        }

        QSqlQuery insertQuery(db);
        insertQuery.prepare(
            "INSERT INTO foods (name, category_id) "
            "VALUES (:name, :category_id);");

        auto insertFood = [&](const QString &foodName, int categoryId) {
            insertQuery.bindValue(":name", foodName);
            insertQuery.bindValue(":category_id", categoryId);
            if (!insertQuery.exec()) {
                qWarning() << "Insert food error:" << insertQuery.lastError().text();
            }
        };

        if (fruitId != -1)
            insertFood("Apple", fruitId);
        if (fastId != -1)
            insertFood("Cheeseburger", fastId);
        if (drinkId != -1)
            insertFood("Latte", drinkId);
    }
}

void MainWindow::setupModelAndView()
{
    model = new QSqlRelationalTableModel(this, db);
    model->setTable("foods");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);

    // Column order: 0 = id, 1 = name, 2 = category_id
    model->setRelation(2, QSqlRelation("categories", "id", "name"));

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Food");
    model->setHeaderData(2, Qt::Horizontal, "Category");

    if (!model->select()) {
        qWarning() << "Model select error:" << model->lastError().text();
    }

    tableView->setModel(model);
    tableView->setItemDelegate(new QSqlRelationalDelegate(tableView));

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked
                               | QAbstractItemView::SelectedClicked);

    tableView->setAlternatingRowColors(true);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setStretchLastSection(true);

    // Optional: hide ID column
    tableView->setColumnHidden(0, true);
}
