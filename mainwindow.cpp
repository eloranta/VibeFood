#include <QDir>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTableView>

#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    // Put the DB file next to the executable
    QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("vibefood.db");

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open())
    {
        qWarning() << "DB open error:" << db.lastError().text();
        return false;
    }

    return true;
}

bool MainWindow::createTables()
{
    QSqlQuery q(db);

    // foods: food_id, name
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS foods ("
            "  food_id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL UNIQUE"
            ");")) {
        qWarning() << "Create foods error:" << q.lastError().text();
        return false;
    }

    // ingredients: ingredient_id, name
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS ingredients ("
            "  ingredient_id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL UNIQUE"
            ");")) {
        qWarning() << "Create ingredients error:" << q.lastError().text();
        return false;
    }

    // pivot: which ingredients each food contains, with optional amount
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS food_ingredients ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  food_id INTEGER NOT NULL,"
            "  ingredient_id INTEGER NOT NULL,"
            "  amount TEXT,"
            "  FOREIGN KEY(food_id) REFERENCES foods(food_id),"
            "  FOREIGN KEY(ingredient_id) REFERENCES ingredients(ingredient_id)"
            ");")) {
        qWarning() << "Create food_ingredients error:" << q.lastError().text();
        return false;
    }

    return true;
}

void MainWindow::seedDataIfEmpty()
{
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) FROM food_ingredients;"))
    {
        qWarning() << "Count food_ingredients error:" << query.lastError().text();
        return;
    }

    addItem("Tortilla", "Tortilla wrap", "1 pc");
    addItem("Tortilla", "Minced meat", "150 g");
    addItem("Tortilla", "Cheese", "50 g");
    addItem("Tortilla", "Lettuce", "some");

    addItem("Salmon soup", "Salmon", "150 g");
    addItem("Salmon soup", "Potatoes", "3 pcs");
    addItem("Salmon soup", "Cream", "200 ml");
    addItem("Salmon soup", "Dill", "to taste");

}

void MainWindow::addItem(const QString &food, const QString &ingredient, const QString &amount)
{
    QSqlQuery query;

    query.prepare("INSERT OR IGNORE INTO foods (name) VALUES (:name)");
    query.bindValue(":name", food);

    if (!query.exec())
    {
        qWarning() << "Inserting error in foods:" << query.lastError().text();
        return;
    }

    int foodId = query.lastInsertId().toInt();
    if (foodId == 0)
    {
        QSqlQuery query2;
        query2.prepare("SELECT food_id FROM foods WHERE name = :name");
        query2.bindValue(":name", food);
        query2.exec();
        if (query2.next())
            foodId = query2.value(0).toInt();
    }

    query.prepare("INSERT OR IGNORE INTO ingredients (name) VALUES (:name)");
    query.bindValue(":name", ingredient);

    if (!query.exec())
    {
        qWarning() << "Inserting error in ingredients:" << query.lastError().text();
        return;
    }

    int ingredientId = query.lastInsertId().toInt();
    if (ingredientId == 0)
    {
        QSqlQuery query2;
        query2.prepare("SELECT ingredient_id FROM ingredients WHERE name = :name");
        query2.bindValue(":name", ingredient);
        query2.exec();
        if (query2.next())
            ingredientId = query2.value(0).toInt();
    }


}

void MainWindow::setupModelAndView()
{
    foodModel = new QSqlTableModel(this, db);
    foodModel->setTable("foods");
    foodModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!foodModel->select())
    {
        qWarning() << "food select error:" << foodModel->lastError().text();
    }

    ingredientsModel = new QSqlTableModel(this, db);
    ingredientsModel->setTable("ingredients");
    ingredientsModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!ingredientsModel->select())
    {
        qWarning() << "ingredients select error:" << ingredientsModel->lastError().text();
    }

    ui->foodView->setModel(foodModel);
//    foodView->setItemDelegate(new QSqlRelationalDelegate(foodView));
    ui->foodView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->foodView->setSelectionMode(QAbstractItemView::SingleSelection);
//    foodView->setSortingEnabled(true);

    ui->ingredientView->setModel(ingredientsModel);
    //    ingredientView->setItemDelegate(new QSqlRelationalDelegate(foodView));
    ui->ingredientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ingredientView->setSelectionMode(QAbstractItemView::SingleSelection);
    //    ingredientView->setSortingEnabled(true);

}



