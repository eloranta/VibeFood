#include "mainwindow.h"

#include <QTableView>
#include <QHeaderView>
#include <QSplitter>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <QItemSelectionModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    foodModel(nullptr),
    ingredientModel(nullptr),
    foodView(new QTableView(this)),
    ingredientView(new QTableView(this))
{
    setWindowTitle("VibeFood");

    if (!setupDatabase()) {
        QMessageBox::critical(this, "DB Error",
                              "Failed to open database.");
    } else {
        initData();
        setupModelsAndViews();
    }

    resize(1000, 600);
}

MainWindow::~MainWindow()
{
    if (db.isOpen())
        db.close();
}

bool MainWindow::setupDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("vibefood.db");

    if (!db.open()) {
        qWarning() << "DB open error:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    // foods table (no categories)
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS foods ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL"
            ");")) {
        qWarning() << "Create foods error:" << query.lastError().text();
        return false;
    }

    // ingredients table
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS ingredients ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name TEXT NOT NULL,"
            "  food_id INTEGER,"
            "  FOREIGN KEY(food_id) REFERENCES foods(id)"
            ");")) {
        qWarning() << "Create ingredients error:" << query.lastError().text();
        return false;
    }

    return true;
}

void MainWindow::initData()
{
    QSqlQuery query(db);

    // --- Seed foods ---
    if (!query.exec("SELECT COUNT(*) FROM foods;")) {
        qWarning() << "Count foods error:" << query.lastError().text();
        return;
    }

    if (query.next() && query.value(0).toInt() == 0) {

        QStringList foods = {
            "Tortilla",
            "Salmon soup",
            "Fried liver",
            "Pasta Bolognese",
            "Cured salmon",
            "Hamburger",
            "Fried mushrooms",
            "Salmon nigiri",
            "Macaroni casserole",
            "Ribs",
            "Pulled Pork"
        };

        QSqlQuery insertFood(db);
        insertFood.prepare("INSERT INTO foods (name) VALUES (:name);");

        for (const QString &food : foods) {
            insertFood.bindValue(":name", food);
            if (!insertFood.exec()) {
                qWarning() << "Insert food error:" << insertFood.lastError().text();
            }
        }
    }

    // --- Seed ingredients ---
    if (!query.exec("SELECT COUNT(*) FROM ingredients;")) {
        qWarning() << "Count ingredients error:" << query.lastError().text();
        return;
    }

    if (query.next() && query.value(0).toInt() == 0) {

        // Lookup food IDs
        QMap<QString,int> foodIds;
        QSqlQuery fq(db);
        fq.exec("SELECT id, name FROM foods;");
        while (fq.next()) {
            foodIds[fq.value(1).toString()] = fq.value(0).toInt();
        }

        QSqlQuery insertIng(db);
        insertIng.prepare(
            "INSERT INTO ingredients (name, food_id) VALUES (:name, :food_id);"
            );

        auto add = [&](QString ing, QString food) {
            if (!foodIds.contains(food)) return;
            insertIng.bindValue(":name", ing);
            insertIng.bindValue(":food_id", foodIds[food]);
            insertIng.exec();
        };

        // Multiple ingredients per food
        add("Tortilla wrap", "Tortilla");
        add("Minced meat", "Tortilla");
        add("Cheese", "Tortilla");
        add("Lettuce", "Tortilla");

        add("Salmon", "Salmon soup");
        add("Potatoes", "Salmon soup");
        add("Cream", "Salmon soup");
        add("Dill", "Salmon soup");

        add("Liver", "Fried liver");
        add("Butter", "Fried liver");
        add("Onion", "Fried liver");

        add("Pasta", "Pasta Bolognese");
        add("Minced meat", "Pasta Bolognese");
        add("Tomato sauce", "Pasta Bolognese");

        add("Salmon", "Cured salmon");
        add("Salt", "Cured salmon");
        add("Sugar", "Cured salmon");
        add("Dill", "Cured salmon");

        add("Bun", "Hamburger");
        add("Beef patty", "Hamburger");
        add("Cheddar", "Hamburger");
        add("Pickles", "Hamburger");

        add("Mushrooms", "Fried mushrooms");
        add("Oil", "Fried mushrooms");
        add("Garlic", "Fried mushrooms");

        add("Rice", "Salmon nigiri");
        add("Salmon", "Salmon nigiri");
        add("Nori", "Salmon nigiri");

        add("Macaroni", "Macaroni casserole");
        add("Minced meat", "Macaroni casserole");
        add("Cheese", "Macaroni casserole");

        add("Pork ribs", "Ribs");
        add("BBQ sauce", "Ribs");

        add("Pulled pork", "Pulled Pork");
        add("BBQ sauce", "Pulled Pork");
        add("Bun", "Pulled Pork");
    }
}

void MainWindow::setupModelsAndViews()
{
    // --- Food model ---
    foodModel = new QSqlTableModel(this, db);
    foodModel->setTable("foods");
    foodModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    foodModel->select();

    foodModel->setHeaderData(0, Qt::Horizontal, "ID");
    foodModel->setHeaderData(1, Qt::Horizontal, "Food");

    // --- Ingredient model ---
    ingredientModel = new QSqlTableModel(this, db);
    ingredientModel->setTable("ingredients");
    ingredientModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    ingredientModel->select();

    ingredientModel->setHeaderData(0, Qt::Horizontal, "ID");
    ingredientModel->setHeaderData(1, Qt::Horizontal, "Ingredient");
    ingredientModel->setHeaderData(2, Qt::Horizontal, "Food ID");

    // --- Views ---
    foodView->setModel(foodModel);
    foodView->setSelectionBehavior(QAbstractItemView::SelectRows);
    foodView->setSelectionMode(QAbstractItemView::SingleSelection);
    foodView->horizontalHeader()->setStretchLastSection(true);
    foodView->setColumnHidden(0, true);

    ingredientView->setModel(ingredientModel);
    ingredientView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ingredientView->setSelectionMode(QAbstractItemView::SingleSelection);
    ingredientView->horizontalHeader()->setStretchLastSection(true);
    ingredientView->setColumnHidden(0, true);
    ingredientView->setColumnHidden(2, true);

    // Layout: splitter
    auto *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(foodView);
    splitter->addWidget(ingredientView);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    // Connect master->detail
    auto *sel = foodView->selectionModel();
    connect(sel, &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onFoodSelectionChanged);

    if (foodModel->rowCount() > 0) {
        foodView->selectRow(0);
        int id = foodModel->data(foodModel->index(0, 0)).toInt();
        updateIngredientsForFood(id);
    }
}

void MainWindow::updateIngredientsForFood(int foodId)
{
    ingredientModel->setFilter(
        QString("food_id = %1").arg(foodId)
        );
    ingredientModel->select();
}

void MainWindow::onFoodSelectionChanged(const QItemSelection &selected,
                                        const QItemSelection &)
{
    if (selected.indexes().isEmpty()) {
        ingredientModel->setFilter("1 = 0");
        ingredientModel->select();
        return;
    }

    QModelIndex idx = selected.indexes().first();
    int foodId = foodModel->data(foodModel->index(idx.row(), 0)).toInt();
    updateIngredientsForFood(foodId);
}
