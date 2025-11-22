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

    connect(ui->btnShowAll, &QPushButton::toggled,
            this, &MainWindow::onShowAllToggled);

    connect(ui->textEdit, &QTextEdit::textChanged,
            this, &MainWindow::onRecipeChanged);   // <-- NEW
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openDatabase()
{
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

    // foods table (new: recipe TEXT)
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS foods ("
            "  food_id INTEGER PRIMARY KEY,"
            "  name TEXT NOT NULL UNIQUE,"
            "  recipe TEXT"
            ");"))
    {
        qWarning() << "Create foods error:" << q.lastError().text();
        return false;
    }

    // Ensure recipe column exists in older DB
    q.exec("PRAGMA table_info(foods)");
    bool hasRecipe = false;
    while (q.next())
    {
        if (q.value(1).toString() == "recipe")
            hasRecipe = true;
    }
    if (!hasRecipe)
        q.exec("ALTER TABLE foods ADD COLUMN recipe TEXT");

    // ingredients
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS ingredients ("
            "  ingredient_id INTEGER PRIMARY KEY,"
            "  name TEXT NOT NULL UNIQUE"
            ");"))
    {
        qWarning() << "Create ingredients error:" << q.lastError().text();
        return false;
    }

    // pivot table
    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS food_ingredients ("
            "  id INTEGER PRIMARY KEY,"
            "  food_id INTEGER NOT NULL,"
            "  ingredient_id INTEGER NOT NULL,"
            "  amount TEXT,"
            "  FOREIGN KEY(food_id) REFERENCES foods(food_id),"
            "  FOREIGN KEY(ingredient_id) REFERENCES ingredients(ingredient_id)"
            ");"))
    {
        qWarning() << "Create food_ingredients error:" << q.lastError().text();
        return false;
    }

    return true;
}

void MainWindow::seedDataIfEmpty()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM food_ingredients;");
    if (query.next() && query.value(0).toInt() != 0)
        return;

    addItem("Tortilla", "Tortilla wrap", "1 pc");
    addItem("Tortilla", "Minced meat", "150 g");
    addItem("Tortilla", "Tomatoes", "some");
    addItem("Tortilla", "Cucumber", "some");
    addItem("Tortilla", "Corn", "50 g");
    addItem("Tortilla", "Turkish youghurt", "some");
    addItem("Tortilla", "Garlic", "1 glove");

    addItem("Salmon soup", "Salmon", "150 g");
    addItem("Salmon soup", "Potatoes", "3 pcs");
    addItem("Salmon soup", "Carrots", "200 ml");
    addItem("Salmon soup", "Onion", "150 g");
    addItem("Salmon soup", "Butter", "3 pcs");
    addItem("Salmon soup", "Cream", "200 ml");
    addItem("Salmon soup", "Dill", "to taste");

    addItem("Fried Liver", "Liver", "1 pc");
    addItem("Fried Liver", "Butter", "1 pc");

    addItem("Pasta Bolognese", "Pancetta", "150 g");
    addItem("Pasta Bolognese", "Onion", "3 pcs");
    addItem("Pasta Bolognese", "Carrots", "200 ml");
    addItem("Pasta Bolognese", "Cellery", "150 g");
    addItem("Pasta Bolognese", "Minced meat", "3 pcs");
    addItem("Pasta Bolognese", "Red Wine", "200 ml");
    addItem("Pasta Bolognese", "Tomato Paste", "to taste");
    addItem("Pasta Bolognese", "Buljong", "to taste");
    addItem("Pasta Bolognese", "Milk", "to taste");
    addItem("Pasta Bolognese", "Spagetti", "to taste");

    addItem("Cured Salmon", "Salmon", "1 pc");

    addItem("Hamburger", "Minced Meat", "1 pc");
    addItem("Hamburger", "Bun", "1 pc");
    addItem("Hamburger", "Cheddar", "1 slice");
    addItem("Hamburger", "Mayonese", "few slices");
    addItem("Hamburger", "Lettuce", "few slices");

    addItem("Fried Mushrooms", "Mushrooms", "300 g");
    addItem("Fried Mushrooms", "Garlic", "300 g");
    addItem("Fried Mushrooms", "Pecorino", "300 g");
    addItem("Fried Mushrooms", "Aurajuusto", "300 g");

    addItem("Salmon nigiri", "Salmon", "2 slices");
    addItem("Salmon nigiri", "Rice", "100 g");
    addItem("Salmon nigiri", "Teriyaki souce", "optional");
    addItem("Salmon nigiri", "Mayonese", "optional");
    addItem("Salmon nigiri", "Wasabi", "optional");
    addItem("Salmon nigiri", "Sesame Seeds", "optional");
    addItem("Salmon nigiri", "Green Onion", "optional");

    addItem("Macaroni Casserole", "Macaroni", "300 g");
    addItem("Macaroni Casserole", "Minced Meat", "300 g");
    addItem("Macaroni Casserole", "Eggs", "300 g");
    addItem("Macaroni Casserole", "Milk", "300 g");
    addItem("Macaroni Casserole", "Cheese", "300 g");

    addItem("Ribs", "Pork ribs", "300 g");
    addItem("Ribs", "BBQ sauce", "50 g");

    addItem("Pulled Pork", "Pulled pork", "150 g");
    addItem("Pulled Pork", "BBQ sauce", "30 g");
    addItem("Pulled Pork", "Bun", "1 pc");}

void MainWindow::addItem(const QString &food, const QString &ingredient, const QString &amount)
{
    QSqlQuery q;

    q.prepare("INSERT OR IGNORE INTO foods (name) VALUES (:n)");
    q.bindValue(":n", food);
    q.exec();

    q.prepare("SELECT food_id FROM foods WHERE name=:n");
    q.bindValue(":n", food);
    q.exec();
    int foodId = q.next() ? q.value(0).toInt() : 0;

    q.prepare("INSERT OR IGNORE INTO ingredients (name) VALUES (:n)");
    q.bindValue(":n", ingredient);
    q.exec();

    q.prepare("SELECT ingredient_id FROM ingredients WHERE name=:n");
    q.bindValue(":n", ingredient);
    q.exec();
    int ingredientId = q.next() ? q.value(0).toInt() : 0;

    q.prepare("INSERT INTO food_ingredients (food_id, ingredient_id, amount)"
              "VALUES (:f,:i,:a)");
    q.bindValue(":f", foodId);
    q.bindValue(":i", ingredientId);
    q.bindValue(":a", amount);
    q.exec();
}

void MainWindow::setupModelAndView()
{
    foodModel = new QSqlTableModel(this, db);
    foodModel->setTable("foods");
    foodModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    foodModel->select();

    foodModel->setHeaderData(1, Qt::Horizontal, "Food");
    foodModel->setHeaderData(2, Qt::Horizontal, "Recipe");

    ui->foodView->setModel(foodModel);
    ui->foodView->hideColumn(0); // food_id
    ui->foodView->hideColumn(2); // hide recipe column
    ui->foodView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->foodView->setSelectionMode(QAbstractItemView::SingleSelection);

    ingredientsModel = new IngredientModelWithCheck(this, db);
    ingredientsModel->setTable("ingredients");
    ingredientsModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    ingredientsModel->select();

    ingredientsModel->setHeaderData(1, Qt::Horizontal, "Ingredient");

    ui->ingredientView->setModel(ingredientsModel);
    ui->ingredientView->hideColumn(1);
    ui->ingredientView->setColumnWidth(0, 35);
    ui->ingredientView->setColumnWidth(3, 120);
    ui->ingredientView->setEditTriggers(QAbstractItemView::AllEditTriggers);

    connect(ui->foodView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onFoodSelectionChanged);

    ui->foodView->selectRow(0);
    onFoodSelectionChanged(foodModel->index(0,0), QModelIndex());
}

void MainWindow::onFoodSelectionChanged(const QModelIndex &current, const QModelIndex &)
{
    if (!current.isValid())
        return;

    int foodId = foodModel->data(foodModel->index(current.row(), 0)).toInt();

    //----------------------------
    // Load recipe text
    //----------------------------
    {
        QSqlQuery q(db);
        q.prepare("SELECT recipe FROM foods WHERE food_id = :id");
        q.bindValue(":id", foodId);
        q.exec();
        if (q.next())
            ui->textEdit->setPlainText(q.value(0).toString());
        else
            ui->textEdit->clear();
    }

    //----------------------------
    // Load ingredient check + amount
    //----------------------------
    QSqlQuery q(db);
    q.prepare("SELECT ingredient_id, amount FROM food_ingredients WHERE food_id=:f");
    q.bindValue(":f", foodId);
    q.exec();

    QSet<int> ids;
    QMap<int, QString> amounts;

    while (q.next())
    {
        ids.insert(q.value(0).toInt());
        amounts.insert(q.value(0).toInt(), q.value(1).toString());
    }

    ingredientsModel->setCurrentFoodId(foodId);
    ingredientsModel->setCheckedRows(ids, amounts);

    onShowAllToggled(ui->btnShowAll->isChecked());
}

void MainWindow::onShowAllToggled(bool showAll)
{
    int rows = ingredientsModel->rowCount();
    for (int r = 0; r < rows; ++r)
    {
        bool checked = ingredientsModel->data(
                                           ingredientsModel->index(r,0),
                                           Qt::CheckStateRole).toInt() == Qt::Checked;

        bool visible = showAll || checked;
        ui->ingredientView->setRowHidden(r, !visible);
    }
}

//-------------------------------------
// SAVE RECIPE TEXT WHEN USER EDITS IT
//-------------------------------------
void MainWindow::onRecipeChanged()
{
    QModelIndex cur = ui->foodView->currentIndex();
    if (!cur.isValid())
        return;

    int foodId = foodModel->data(foodModel->index(cur.row(), 0)).toInt();
    QString text = ui->textEdit->toPlainText();

    QSqlQuery q(db);
    q.prepare("UPDATE foods SET recipe=:r WHERE food_id=:id");
    q.bindValue(":r", text);
    q.bindValue(":id", foodId);
    q.exec();
}
