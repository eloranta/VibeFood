#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTextEdit>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->horizontalLayout_2->setStretch(0, 1);
    ui->horizontalLayout_2->setStretch(1, 1);
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
    foodModel->setSort(1, Qt::AscendingOrder);
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
    ui->foodView->sortByColumn(1, Qt::AscendingOrder);
    ui->foodView->setColumnWidth(0, 60);
    ui->foodView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->foodView->hideColumn(2);
    ui->foodView->selectRow(0);

    ingredientModel = new QSqlTableModel(this, db);
    ingredientModel->setTable("food_view");
    ingredientModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    ingredientModel->setHeaderData(2, Qt::Horizontal, "Ingredient");
    ingredientModel->setHeaderData(3, Qt::Horizontal, "Amount");

    ui->ingredientView->setModel(ingredientModel);
    ui->ingredientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ingredientView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->ingredientView->setSortingEnabled(true);
    ui->ingredientView->setColumnWidth(0, 60);
    ui->ingredientView->hideColumn(1);
    ui->ingredientView->horizontalHeader()->setStretchLastSection(true);
    ui->ingredientView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->ingredientView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    connect(foodModel, &QSqlTableModel::dataChanged, ingredientModel, &QSqlTableModel::select);
    connect(foodModel, &QSqlTableModel::rowsInserted, ingredientModel, &QSqlTableModel::select);
    connect(foodModel, &QSqlTableModel::rowsRemoved, ingredientModel, &QSqlTableModel::select);

    if (auto selModel = ui->foodView->selectionModel())
    {
        connect(selModel, &QItemSelectionModel::currentRowChanged, this,
                [this](const QModelIndex &current, const QModelIndex &) {
                    const int foodId = current.sibling(current.row(), 0).data().toInt();
                    setIngredientFilterForFood(foodId);
                    updatingRecipeText = true;
                    ui->textEdit->setPlainText(current.isValid()
                                                   ? current.sibling(current.row(), 2).data().toString()
                                                   : QString());
                    updatingRecipeText = false;
                });
    }

    const QModelIndex current = ui->foodView->selectionModel()
            ? ui->foodView->selectionModel()->currentIndex()
            : QModelIndex();
    setIngredientFilterForFood(current.isValid() ? current.sibling(current.row(), 0).data().toInt() : -1);
    updatingRecipeText = true;
    ui->textEdit->setPlainText(current.isValid()
                                   ? current.sibling(current.row(), 2).data().toString()
                                   : QString());
    updatingRecipeText = false;

    connect(ui->textEdit, &QTextEdit::textChanged, this, [this]() {
        if (updatingRecipeText)
            return;

        const auto *selection = ui->foodView->selectionModel();
        if (!selection)
            return;

        const QModelIndex currentIdx = selection->currentIndex();
        if (!currentIdx.isValid())
            return;

        const QModelIndex recipeIdx = foodModel->index(currentIdx.row(), 2);
        updatingRecipeText = true;
        foodModel->setData(recipeIdx, ui->textEdit->toPlainText());
        updatingRecipeText = false;
    });

    connect(ui->addFood, &QPushButton::clicked, this, &MainWindow::addFood);
    connect(ui->deleteFood, &QPushButton::clicked, this, &MainWindow::deleteFood);
    connect(ui->addIngredient, &QPushButton::clicked, this, &MainWindow::addIngredient);
    connect(ui->deleteIngredient, &QPushButton::clicked, this, &MainWindow::deleteIngredient);
}

void MainWindow::setIngredientFilterForFood(int foodId)
{
    if (!ingredientModel)
        return;

    if (foodId <= 0)
    {
        ingredientModel->setFilter(QString());
    }
    else
    {
        ingredientModel->setFilter(QStringLiteral("food_id=%1").arg(foodId));
    }
    if (ingredientModel)
        ingredientModel->select();
}

void MainWindow::addFood()
{
    const QString name = "New Food";

    QSqlQuery query(db);
    query.prepare("INSERT INTO foods (name, recipe) VALUES (:n, :r);");
    query.bindValue(":n", name);
    query.bindValue(":r", QString());
    if (!query.exec())
    {
        QMessageBox::warning(this, "Add food failed", query.lastError().text());
        return;
    }

    const int newId = query.lastInsertId().isValid() ? query.lastInsertId().toInt() : -1;

    foodModel->select();

    int targetRow = -1;
    if (newId > 0)
    {
        for (int row = 0; row < foodModel->rowCount(); ++row)
        {
            if (foodModel->index(row, 0).data().toInt() == newId)
            {
                targetRow = row;
                break;
            }
        }
    }

    if (targetRow < 0)
    {
        for (int row = 0; row < foodModel->rowCount(); ++row)
        {
            if (foodModel->index(row, 1).data().toString() == name)
            {
                targetRow = row;
                break;
            }
        }
    }

    if (targetRow >= 0)
    {
        const QModelIndex idx = foodModel->index(targetRow, 0);
        ui->foodView->selectRow(targetRow);
        ui->foodView->scrollTo(idx);
    }
}

void MainWindow::deleteFood()
{
    const auto *selection = ui->foodView->selectionModel();
    if (!selection)
        return;

    const QModelIndex currentIdx = selection->currentIndex();
    if (!currentIdx.isValid())
        return;

    const int foodId = foodModel->index(currentIdx.row(), 0).data().toInt();
    if (foodId <= 0)
        return;

    QSqlQuery query(db);
    query.prepare("DELETE FROM food_ingredients WHERE food_id = :id;");
    query.bindValue(":id", foodId);
    if (!query.exec())
    {
        QMessageBox::warning(this, "Delete food failed", query.lastError().text());
        return;
    }

    query.prepare("DELETE FROM foods WHERE food_id = :id;");
    query.bindValue(":id", foodId);
    if (!query.exec())
    {
        QMessageBox::warning(this, "Delete food failed", query.lastError().text());
        return;
    }

    foodModel->select();
    ingredientModel->select();

    const int rowCount = foodModel->rowCount();
    if (rowCount == 0)
    {
        setIngredientFilterForFood(-1);
        updatingRecipeText = true;
        ui->textEdit->clear();
        updatingRecipeText = false;
        return;
    }

    const int targetRow = std::clamp(currentIdx.row(), 0, rowCount - 1);
    ui->foodView->selectRow(targetRow);
    ui->foodView->scrollTo(foodModel->index(targetRow, 0));
}

void MainWindow::addIngredient()
{
    const auto *foodSelection = ui->foodView->selectionModel();
    if (!foodSelection)
        return;

    const QModelIndex currentIdx = foodSelection->currentIndex();
    if (!currentIdx.isValid())
        return;

    const int foodId = foodModel->index(currentIdx.row(), 0).data().toInt();
    if (foodId <= 0)
        return;

    const QString ingredientName = "New Ingredient";

    QSqlQuery query(db);
    query.prepare("INSERT OR IGNORE INTO ingredients (name) VALUES (:n);");
    query.bindValue(":n", ingredientName);
    if (!query.exec())
    {
        QMessageBox::warning(this, "Add ingredient failed", query.lastError().text());
        return;
    }

    query.prepare(
        "INSERT OR REPLACE INTO food_ingredients (food_id, ingredient_id, amount) "
        "VALUES (:f, (SELECT ingredient_id FROM ingredients WHERE name = :n), '');");
    query.bindValue(":f", foodId);
    query.bindValue(":n", ingredientName);
    if (!query.exec())
    {
        QMessageBox::warning(this, "Add ingredient failed", query.lastError().text());
        return;
    }

    ingredientModel->select();

    int targetRow = -1;
    for (int row = 0; row < ingredientModel->rowCount(); ++row)
    {
        const int rowFoodId = ingredientModel->index(row, 0).data().toInt();
        const QString rowName = ingredientModel->index(row, 2).data().toString();
        if (rowFoodId == foodId && rowName == ingredientName)
        {
            targetRow = row;
            break;
        }
    }

    if (targetRow >= 0)
    {
        const QModelIndex idx = ingredientModel->index(targetRow, 0);
        ui->ingredientView->selectRow(targetRow);
        ui->ingredientView->scrollTo(idx);
    }
}

void MainWindow::deleteIngredient()
{
    const auto *selection = ui->ingredientView->selectionModel();
    if (!selection)
        return;

    const QModelIndex currentIdx = selection->currentIndex();
    if (!currentIdx.isValid())
        return;

    const int foodId = ingredientModel->index(currentIdx.row(), 0).data().toInt();
    const QString ingredientName = ingredientModel->index(currentIdx.row(), 2).data().toString();
    if (foodId <= 0 || ingredientName.isEmpty())
        return;

    QSqlQuery query(db);
    query.prepare(
        "DELETE FROM food_ingredients "
        "WHERE food_id = :f "
        "  AND ingredient_id = (SELECT ingredient_id FROM ingredients WHERE name = :n LIMIT 1);");
    query.bindValue(":f", foodId);
    query.bindValue(":n", ingredientName);
    if (!query.exec())
    {
        QMessageBox::warning(this, "Delete ingredient failed", query.lastError().text());
        return;
    }

    ingredientModel->select();

    const int rowCount = ingredientModel->rowCount();
    if (rowCount == 0)
        return;

    const int targetRow = std::clamp(currentIdx.row(), 0, rowCount - 1);
    ui->ingredientView->selectRow(targetRow);
    ui->ingredientView->scrollTo(ingredientModel->index(targetRow, 0));
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

    query.exec("DROP VIEW IF EXISTS food_view;");

    if (!query.exec(
            "CREATE VIEW IF NOT EXISTS food_view AS "
            "SELECT "
            "  f.food_id, "
            "  f.name AS food_name, "
            "  i.name AS ingredient_name, "
            "  fi.amount "
            "FROM food_ingredients fi "
            "JOIN foods f ON fi.food_id = f.food_id "
            "JOIN ingredients i ON fi.ingredient_id = i.ingredient_id "
            "ORDER BY f.name, i.name;"))
    {
        qWarning() << "Create food_view error:" << query.lastError().text();
        return false;
    }

    query.exec("DROP TRIGGER IF EXISTS update_food_view;");

    if (!query.exec(
            "CREATE TRIGGER IF NOT EXISTS update_food_view "
            "INSTEAD OF UPDATE ON food_view "
            "BEGIN "
            "  UPDATE foods "
            "    SET name = NEW.food_name "
            "    WHERE food_id = NEW.food_id; "
            "  UPDATE ingredients "
            "    SET name = NEW.ingredient_name "
            "    WHERE ingredient_id = ("
            "      SELECT fi.ingredient_id "
            "      FROM food_ingredients fi "
            "      JOIN ingredients i ON fi.ingredient_id = i.ingredient_id "
            "      WHERE fi.food_id = OLD.food_id "
            "        AND i.name = OLD.ingredient_name "
            "      LIMIT 1"
            "    ); "
            "  UPDATE food_ingredients "
            "    SET amount = NEW.amount "
            "    WHERE food_id = NEW.food_id "
            "      AND ingredient_id = ("
            "        SELECT fi.ingredient_id "
            "        FROM food_ingredients fi "
            "        JOIN ingredients i ON fi.ingredient_id = i.ingredient_id "
            "        WHERE fi.food_id = OLD.food_id "
            "          AND i.name = OLD.ingredient_name "
            "        LIMIT 1"
            "      ); "
            "END;"))
    {
        qWarning() << "Create update_food_view trigger error:" << query.lastError().text();
        return false;
    }

    return true;
}
