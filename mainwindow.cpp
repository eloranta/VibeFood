#include <QDir>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTableView>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "IngredientModelWithCheck.h"

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
            "  food_id INTEGER PRIMARY KEY,"
            "  name TEXT NOT NULL UNIQUE"
            ");")) {
        qWarning() << "Create foods error:" << q.lastError().text();
        return false;
    }

    // ingredients: ingredient_id, name
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS ingredients ("
            "  ingredient_id INTEGER PRIMARY KEY,"
            "  name TEXT NOT NULL UNIQUE"
            ");")) {
        qWarning() << "Create ingredients error:" << q.lastError().text();
        return false;
    }

    // pivot: which ingredients each food contains, with optional amount
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS food_ingredients ("
            "  id INTEGER PRIMARY KEY,"
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

    if (!query.next() || query.value(0).toInt() != 0)
        return;

    addItem("Tortilla", "Tortilla wrap", "1 pc");
    addItem("Tortilla", "Minced meat", "150 g");
    addItem("Tortilla", "Cheese", "50 g");
    addItem("Tortilla", "Lettuce", "some");

    addItem("Salmon soup", "Salmon", "150 g");
    addItem("Salmon soup", "Potatoes", "3 pcs");
    addItem("Salmon soup", "Cream", "200 ml");
    addItem("Salmon soup", "Dill", "to taste");

    addItem("Hamburger", "Bun", "1 pc");
    addItem("Hamburger", "Beef patty", "1 pc");
    addItem("Hamburger", "Cheddar", "1 slice");
    addItem("Hamburger", "Pickles", "few slices");

    addItem("Salmon nigiri", "Rice", "100 g");
    addItem("Salmon nigiri", "Salmon", "2 slices");
    addItem("Salmon nigiri", "Nori", "optional");

    addItem("Ribs", "Pork ribs", "300 g");
    addItem("Ribs", "BBQ sauce", "50 g");

    addItem("Pulled Pork", "Pulled pork", "150 g");
    addItem("Pulled Pork", "BBQ sauce", "30 g");
    addItem("Pulled Pork", "Bun", "1 pc");

}

void MainWindow::addItem(const QString &food, const QString &ingredient, const QString &amount)
{
    QSqlQuery query1;

    query1.prepare("INSERT OR IGNORE INTO foods (name) VALUES (:name)");
    query1.bindValue(":name", food);

    if (!query1.exec())
    {
        qWarning() << "Inserting error in foods:" << query1.lastError().text();
        return;
    }

    int foodId = 0;
    query1.prepare("SELECT food_id FROM foods WHERE name = :name");
    query1.bindValue(":name", food);
    query1.exec();
    if (query1.next())
        foodId = query1.value(0).toInt();

    QSqlQuery query2;

    query2.prepare("INSERT OR IGNORE INTO ingredients (name) VALUES (:name)");
    query2.bindValue(":name", ingredient);

    if (!query2.exec())
    {
        qWarning() << "Inserting error in ingredients:" << query2.lastError().text();
        return;
    }

    int ingredientId = 0;
    query2.prepare("SELECT ingredient_id FROM ingredients WHERE name = :name");
    query2.bindValue(":name", ingredient);
    query2.exec();
    if (query2.next())
        ingredientId = query2.value(0).toInt();

    QSqlQuery query3;
    query3.prepare(
        "INSERT INTO food_ingredients (food_id, ingredient_id, amount) "
        "VALUES (:food_id, :ingredient_id, :amount);");
    query3.bindValue(":food_id", foodId);
    query3.bindValue(":ingredient_id", ingredientId);
    query3.bindValue(":amount", amount);
    if (!query3.exec())
    {
        qWarning() << "Insert food_ingredients error:" << query3.lastError().text();
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
    foodModel->setHeaderData(1, Qt::Horizontal, "Food");


    ingredientsModel = new IngredientModelWithCheck(this, db);
    ingredientsModel->setTable("ingredients");
    ingredientsModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    if (!ingredientsModel->select())
    {
        qWarning() << "ingredients select error:" << ingredientsModel->lastError().text();
    }
    ingredientsModel->setHeaderData(1, Qt::Horizontal, "Incredient");

    ui->foodView->setStyleSheet(
        "QTableView::item:selected {"
        "    background-color: lightblue;"
        "    color: black;"
        "}"
        "QTableView::item:selected:active {"
        "    background-color: lightblue;"
        "}"
        "QTableView::item:selected:!active {"
        "    background-color: #cce9ff;"  // slightly lighter
        "}"
        );

    ui->foodView->setModel(foodModel);
//    foodView->setItemDelegate(new QSqlRelationalDelegate(foodView));
    ui->foodView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->foodView->setSelectionMode(QAbstractItemView::SingleSelection);
//    foodView->setSortingEnabled(true);
    ui->foodView->hideColumn(0);

    ui->ingredientView->setModel(ingredientsModel);
    //    ingredientView->setItemDelegate(new QSqlRelationalDelegate(foodView));
    ui->ingredientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ingredientView->setSelectionMode(QAbstractItemView::SingleSelection);
    //    ingredientView->setSortingEnabled(true);
    ui->ingredientView->hideColumn(1);
}



