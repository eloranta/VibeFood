#include "mainwindow.h"

#include <QTableView>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRelationalDelegate>
#include <QMessageBox>
#include <QHeaderView>

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
    // Use SQLite (file in working directory)
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("vibefood.db");

    if (!db.open()) {
        qWarning("Error: %s", qPrintable(db.lastError().text()));
        return false;
    }

    QSqlQuery query(db);

    // Create categories table
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS categories ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL UNIQUE"
            ");")) {
        qWarning("Create categories error: %s",
                 qPrintable(query.lastError().text()));
        return false;
    }

    // Create foods table with foreign key to categories
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS foods ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL,"
            "  calories INTEGER,"
            "  category_id INTEGER,"
            "  FOREIGN KEY(category_id) REFERENCES categories(id)"
            ");")) {
        qWarning("Create foods error: %s",
                 qPrintable(query.lastError().text()));
        return false;
    }

    return true;
}

void MainWindow::initData()
{
    QSqlQuery query(db);

    // Insert categories if empty
    if (!query.exec("SELECT COUNT(*) FROM categories;")) {
        return;
    }
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec("INSERT INTO categories (name) VALUES ('Fruit');");
        query.exec("INSERT INTO categories (name) VALUES ('Fast Food');");
        query.exec("INSERT INTO categories (name) VALUES ('Drink');");
    }

    // Insert sample foods if empty
    if (!query.exec("SELECT COUNT(*) FROM foods;")) {
        return;
    }
    if (query.next() && query.value(0).toInt() == 0) {
        // Get category ids
        int fruitId = -1, fastId = -1, drinkId = -1;
        QSqlQuery catQuery(db);
        catQuery.exec("SELECT id, name FROM categories;");
        while (catQuery.next()) {
            const QString name = catQuery.value(1).toString();
            int id = catQuery.value(0).toInt();
            if (name == "Fruit") fruitId = id;
            else if (name == "Fast Food") fastId = id;
            else if (name == "Drink") drinkId = id;
        }

        query.prepare("INSERT INTO foods (name, calories, category_id) "
                      "VALUES (?, ?, ?);");

        query.addBindValue("Apple");
        query.addBindValue(95);
        query.addBindValue(fruitId);
        query.exec();

        query.addBindValue("Cheeseburger");
        query.addBindValue(303);
        query.addBindValue(fastId);
        query.exec();

        query.addBindValue("Latte");
        query.addBindValue(190);
        query.addBindValue(drinkId);
        query.exec();
    }
}

void MainWindow::setupModelAndView()
{
    model = new QSqlRelationalTableModel(this, db);
    model->setTable("foods");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);

    // Column indexes: 0=id, 1=name, 2=calories, 3=category_id
    model->setRelation(3, QSqlRelation("categories", "id", "name"));
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Food");
    model->setHeaderData(2, Qt::Horizontal, "Calories");
    model->setHeaderData(3, Qt::Horizontal, "Category");

    if (!model->select()) {
        qWarning("Model select error: %s",
                 qPrintable(model->lastError().text()));
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
    tableView->setColumnHidden(0, true); // hide ID if you want
}
